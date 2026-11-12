# String Handling

This guide introduces LPCVM's string storage, compares measured assignment costs,
and explains how to write LPC string code that uses the available optimizations.
Bytecode, ownership APIs, and compiler implementation details are in the
[String Runtime Reference](string-runtime.md).

## Representations

LPC strings are byte sequences with explicit lengths. Empty strings and embedded
NUL bytes are supported; concatenation does not interpret encoding or perform
Unicode normalization. LPC code uses the `string` type for all stored
representations; the runtime manages their ownership and conversion.

| Representation | Storage and ownership | Typical use |
|----------------|-----------------------|-------------|
| `LpcStringRef` | Reference-counted immutable bytes, interned by the runtime. Copying a reference shares the existing allocation. | Literals, global string values, and locals or parameters holding shared references. |
| `LpcBufferRef` | Reference-counted mutable buffer from a runtime pool. Writes reuse unshared capacity or use another buffer when the old value is shared. | Computed values stored in scalar `string` locals, with local buffers enabled. |
| `TemporaryString` | Offset and length into the evaluation stack's byte buffer. Copying a temporary duplicates its bytes; releasing it may move later bytes and repair offsets. | Concatenation results, temporary-valued arguments and returns, and computed locals when local buffers are disabled. |
| String accumulator | Mutable workspace owned by one execution invocation; not a stored LPC value or an `SValue`. | Building eligible concatenation chains before producing a temporary or finishing into a local buffer. |

A declaration alone does not select storage: `string x = "text";` holds a shared
reference, while storing a temporary result in `x` normally creates or reuses a
local buffer. Passing or returning a string can preserve a reference or copy a
temporary, depending on the value. Local buffers and the accumulator are enabled
by default; their behavior is described in the [programming guide](#string-handling-guide-for-lpc-programmers).

## SValue Variant Assignment Costs

Assignment cost depends on the operation actually performed, not just the
source and destination's declared LPC types. The measurements below group reference stores, buffer writes,
and interning by their runtime behavior. Within each group, rows are ordered by measured 4-KiB cost.

### Measurement scope

Measured on 2026-11-12 in a CMake `Release` build on an Intel Core i7-13700:
79 configurations, five randomly interleaved repetitions each, no validation
errors. Values are **median CPU nanoseconds per operation**, rounded to whole
nanoseconds (reported batch time divided by 32). The columns denote source byte
length; for merging, they denote each operand's length. See the
[benchmark workflow](development.md#string-assignment-and-release) and
[measurement data](string-assignment-costs.csv) for all sizes and variability.

`BM_StringAssign*` calls runtime primitives directly. Stores include source
expression cleanup; fixture setup, source production, validation, destruction,
and bytecode dispatch are excluded. Each fixture has its own fabric and small
intern table. New-buffer cases start with a fresh pool; reuse starts with enough
capacity. These timings estimate the selected operation, **not the total cost of
the LPC snippets**. The median coefficient of variation across cases was 1.4%;
the 4-KiB temporary-copy case had 30.4% variation and is less precise.

### LPC example conventions

Unless noted, both local buffers and the string accumulator are enabled. Each
snippet is independent; `g` is a global `string`. The following helper guarantees
a temporary return value, unlike a function returning a literal or a reference:

```lpc
string temp() { return "a" + "b"; }
```

Its return expression uses `F_STR_FINISH`, producing a `TemporaryString`.
A call on the right-hand side of `x = temp()` is stored normally; it does not
qualify for the accumulator's `F_STR_FINISH_LOCAL` optimization. The helper's
short payload illustrates representation only; benchmarks use the byte lengths
shown in the columns.

### Reference stores

| Operation | Benchmark suffix | 16 B (ns) | 4 KiB (ns) | LPC example and timed step |
|-----------|------------------|----------:|-----------:|----------------------------|
| Shared reference replaces shared reference | `SharedRef` | 23 | 27 | `string x = "old"; x = "new";` — final store retains the source reference and releases the old one. |
| Buffer reference replaces sole-owner buffer | `BufferRef` | 25 | 31 | `string x = temp(); string y = temp(); x = y;` — final store shares `y` and returns the old `x` buffer to its pool. |
| Shared reference replaces sole-owner buffer | `SharedToBuffer` | 26 | 32 | `string x = temp(); x = "new";` — final store replaces the buffer with a shared reference; no bytes copied into a buffer. |

Reference stores remain cheap as payload size grows. The `SharedToBuffer` name
refers to the destination **before** assignment; its result is `LpcStringRef`.
Likewise, assigning a literal to a temporary-valued parameter stores a shared
reference and releases the old temporary. It does not append another temporary.
That old temporary's cleanup can add the costs described below.

### Temporary sources stored into local buffers

| Operation | Benchmark suffix | 16 B (ns) | 4 KiB (ns) | LPC example and timed step |
|-----------|------------------|----------:|-----------:|----------------------------|
| Reuse sole-owner buffer capacity | `TempReuse` | 26 | 86 | `string x = temp(); x = temp();` — second store copies into `x`'s existing buffer. |
| Copy-on-write with old buffer retained | `TempCow` | 46 | 117 | `string x = temp(); string saved = x; x = temp();` — final store acquires another buffer; `saved` retains the old bytes. |
| Initialize an empty local slot | `TempInit` | 50 | 125 | `string x = temp();` — store acquires a buffer and copies the returned temporary. |
| Upgrade a temporary-valued parameter | `TempUpgrade` | 57 | 179 | `void replace(string p) { p = temp(); }`, called as `replace(temp());` — store acquires a buffer and releases the older parameter temporary. |

Reuse is cheaper than first allocation. Parameter upgrade also removes an older,
non-tail temporary while the source still occupies the buffer. Passing a literal
as `replace("expr")` would instead start with a shared-string parameter and would
not exercise that cleanup. Only temporary sources take the allocation/reuse
branch of `F_STORE_LOCAL_BUFFER`; reference sources are stored by reference.

### Sources interned into shared storage

| Operation | Benchmark suffix | 16 B (ns) | 4 KiB (ns) | LPC example and condition |
|-----------|------------------|----------:|-----------:|---------------------------|
| Temporary, intern hit | `TempInternHit` | 21 | 88 | `g = temp();` — identical complete bytes already exist in the intern table. |
| Buffer, intern hit | `BufferInternHit` | 23 | 93 | `string x = temp(); g = x;` — final store finds identical bytes already interned. |
| Temporary, intern miss | `TempInternMiss` | 36 | 505 | `g = temp();` — complete result bytes are absent from the intern table. |
| Buffer, intern miss | `BufferInternMiss` | 43 | 506 | `string x = temp(); g = x;` — final store inserts previously absent bytes. |

A miss costs about 5.5–5.7 times a hit at 4 KiB in these fixtures. Hit/miss status
is runtime state, not a property of the assignment syntax. For example, repeating
`g = temp()` will hit after the first store while the intern entry remains alive.
Temporary cleanup here is at the tail; non-tail cleanup can be much more costly.
The measurements do not support ranking temporary sources as inherently more
expensive than buffer sources when interning under equivalent conditions.

### Temporary copying and concatenation

| Operation | Benchmark suffix | 16 B (ns) | 4 KiB (ns) | LPC example and condition |
|-----------|------------------|----------:|-----------:|---------------------------|
| Merge contiguous temporary descriptors | `TempMerge` | 15 | 18 | `return temp() + temp();` — outer binary ADD merges nonempty, adjacent temporary operands ending at the buffer tail. |
| Copy a temporary over an older temporary | `TempCopy` | 34 | 397 | With local buffers **disabled**: `string x = temp(); x = temp();` — second store copies source bytes, then releases the older temporary. |

Merging is concatenation, not assignment, and was the fastest measured operation.
Calls prevent accumulator lowering of the outer ADD in the example; the fast
path still requires the actual contiguous buffer layout. `a + b` with declared
string variables normally uses the accumulator and does not illustrate this
path. Empty temporary views do not satisfy the contiguous-buffer fast-path check;
the zero-byte merge benchmark uses the fallback and measured 48 ns.

Temporary copying scales with payload and can include buffer growth and
compaction. The 397-ns result has high variability, so it should not be used to
establish a precise ordering against other expensive paths. Eligible local
concatenations using `F_STR_FINISH_LOCAL` swap storage instead; they are covered
by `BM_StringLocalConcatAccumulatorBuffer`, not these assignment measurements.

### Cleanup can dominate the store

`BM_StringTempRelease` isolates replacement of a temporary by an integer. For a
16-byte released value:

| Trailing bytes | Other stack slots | CPU ns/release | Path |
|---------------:|------------------:|---------------:|------|
| 0 | 0 | 16 | Tail truncation |
| 0 | 4096 | 26 | Tail truncation; no offset scan |
| 16 | 0 | 19 | Small non-tail compaction |
| 16 | 4096 | 1339 | Non-tail compaction and offset scan |
| 4096 | 4096 | 1863 | Larger compaction and offset scan |

The worst cases depend on trailing bytes and live stack slots, not just the
released string's length. LPC syntax alone does not establish that layout:
assignments and returns change temporary positions. These isolated cleanup
numbers explain the risk of interleaved temporaries; they are not an additional
fixed surcharge to add to every assignment row, which already includes its own
fixture's cleanup.

## String Handling Guide for LPC Programmers

The examples below assume the default compiler options: local string buffers
and the string accumulator are both enabled. Storage changes are automatic;
LPC programmers do not allocate, release, or select the internal representations.

### Use declared string locals for string work

Declare values as `string` when that is their intended type. The compiler can
recognize concatenations of string literals and declared scalar string variables
and build the chain with its accumulator:

```lpc
string label(string name, string suffix) {
    string result;
    result = "Hello, " + name + suffix;
    return result;
}
```

Here the chain finishes directly into `result`'s local buffer. This avoids
creating a separate completed temporary and then copying it into the local.
A chain using `mixed` variables still uses ordinary string addition when its
runtime operands are strings, but is not eligible for this optimization.

### Express a known concatenation as one chain

When the pieces are already available, write `prefix + name + suffix` as one
expression. Parentheses do not prevent optimization: `(prefix + name) + suffix`
is flattened in operand order. A literal or string variable can be a term;
function calls and assignments inside the expression prevent the whole chain
from using the accumulator, though eligible subexpressions can still use it.

For example, `return temp() + temp();` from the cost comparison uses ordinary
addition for the outer `+`. It may benefit from the contiguous-temporary merge,
but that depends on runtime layout. Do not add function calls just to try to
obtain this fast path: the merge timing excludes producing its operands.

### Reuse a local, but account for a growing result

Local buffer capacity can be reused across assignments when no other value
shares the buffer. Typed local `+=` also supports accumulator lowering when its
right-hand side contains eligible string terms:

```lpc
string repeat(string piece, int count) {
    string result = "";
    for (int i = 0; i < count; i++)
        result += piece;
    return result;
}
```

This is supported syntax, but repeated append is not guaranteed constant-time.
Each eligible `+=` starts by copying the current result into the accumulator,
then appends the new piece and swaps the completed storage into the local.
As the result grows, repeated prefix copying can make the total work quadratic
in the number of fixed-size pieces. For a known set of pieces, a single chain
avoids rebuilding that prefix across multiple statements. Measure workloads
that build large results in loops; buffer reuse alone does not remove byte-copy
costs.

### Copies preserve string values

Assigning a buffer-backed local to another local shares its current storage.
Subsequent writes preserve the other variable's value through copy-on-write:

```lpc
string snapshot() {
    string current = "a" + "b";
    string saved = current;
    current += "c";
    return saved; // "ab"; current is "abc"
}
```

Keep a copy when you need a snapshot. While that copy remains alive, a write
cannot reuse the shared buffer in place and may need another buffer. A simple
reference assignment is inexpensive; the later write is where sharing can add
cost. Assigning a literal to `current` instead replaces its reference without
copying the literal into the old buffer.

### Treat function arguments and returns according to their values

A function call does not necessarily produce a temporary string:

```lpc
string literal_value() { return "ab"; }
string computed_value() { return "a" + "b"; }
string identity(string value) { return value; }
```

`literal_value()` returns a shared reference. `computed_value()` returns a
stack-owned temporary. `identity()` preserves shared or buffer references, but
copies temporary bytes as needed for loads and returns. A `string` parameter
can initially hold any of these stored representations.

Storing `computed_value()` into a typed local normally copies it into a buffer.
Replacing a temporary-valued parameter with another temporary result creates a
buffer on the first such assignment; later stores can reuse it if it remains
unshared. Assigning a literal or another reference instead stores that reference
directly. Ordinary LPC code needs no manual lifetime management for any of these
cases.

### Build locally before storing retained state

Global string stores retain shared references. Storing a buffer or temporary
there looks up its bytes in the runtime's intern table and may allocate a new
shared string. If only the completed value needs to be visible in object state,
build it locally and store it once:

```lpc
string message;

void update_message(string name, string suffix) {
    string next = "Hello, " + name + suffix;
    message = next;
}
```

The local construction uses the accumulator and local buffer; the final global
store interns the completed bytes. Assigning an already shared value, such as a
literal or another global string, copies its reference instead. Identical
computed bytes can hit the intern table on later stores while that entry remains
alive; different bytes can miss. The measured hit/miss difference is substantial
for larger strings. Keep intermediate global stores when program behavior
requires them to be visible.

### Profile the whole operation before changing code

Use the assignment tables to identify likely costs, then measure representative
LPC execution. Short strings can make several paths similarly cheap; larger
strings expose copying and interning costs. Temporary cleanup also depends on
other live stack values, so a source expression alone cannot establish its cost.

The [string benchmarks](development.md#string-assignment-and-release) isolate
assignment and release. The [concatenation benchmarks](development.md#bytecode)
include loads, string construction, stores, and cleanup under all four compiler
option combinations. They are a better starting point when comparing complete
concatenation strategies. Compiler options are embedding configuration, not LPC
syntax; see [Compiler Lowering](string-runtime.md#compiler-lowering) for details.
