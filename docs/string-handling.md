# String Handling

This document describes the LPCVM string representation, ownership, bytecode,
and compiler lowering. See [vm-architecture.md](vm-architecture.md) for call
frames and [development.md](development.md#bytecode) for benchmark workflows.

## Representations

Strings are byte sequences with explicit lengths. Empty strings and embedded
NUL bytes are supported; concatenation does not interpret encoding or perform
Unicode normalization.

| Representation | Owner | Purpose |
|----------------|-------|---------|
| `LpcStringRef` | Reference-counted shared storage | Constants, object state, and other retained strings. |
| `LpcBufferRef` | Reference-counted, fabric-pooled mutable storage with copy-on-write | Scalar `string` locals by default; see [Local String Buffers](#local-string-buffers). |
| `TemporaryString` | One `DataStack` | Intermediate values, temporary locals, arguments, and returns. |
| String accumulator | One `Execution::execute()` invocation | Mutable storage for a concatenation chain; not an `SValue`. |

### Shared Strings

`LpcStringRef` refers to immutable `LpcString` bytes. `ManagedRuntime` interns
strings by their full byte sequence, including embedded NULs. Copying a reference
retains the existing allocation rather than duplicating bytes.

Each shared string uses one allocation for the reference-count control block,
string metadata, bytes, and trailing NUL. The explicit length, not that trailing
NUL, determines its contents. The runtime's intern table holds a strong reference;
`sweep_shared_strings()` removes entries referenced only by the table. Other
owners, including frozen constant pools and object state, retain strings
independently of the table's lifetime.

`F_CONST_STR` loads an owning reference from the program's constant pool.
Shared-string allocation and interning go through the runtime fabric.

### Stack-Owned Temporaries

`DataStack` stores temporary bytes in a contiguous `std::string`, `cat_buffer_`.
Each `TemporaryString` slot records a `uint32_t` byte offset and length into that
buffer. It is not an owning pointer and cannot be transferred directly between
stacks. Raw `push()` and `assign()` reject temporary-string descriptors.

Appending a temporary copies its bytes to the buffer's end and pushes a new
descriptor. Existing offsets need no repair. Buffer growth can still reallocate
and copy existing bytes. Temporary length and total buffer size must fit in
`uint32_t`.

Temporary values are immutable from the VM's perspective. `push_copy()` and
`assign_copy()` duplicate their bytes; slots do not share temporary ranges.
`fetch_temp_string()` returns a borrowed view that must not survive buffer
mutation. Empty temporaries have no meaningful offset and yield an empty view.

## Loads, Stores, and Returns

Local addresses are relative to the frame pointer; object-state addresses use
the execution's state offset.

| Operation | Shared String | Temporary String |
|-----------|---------------|------------------|
| `F_LOAD_LOCAL` | Copy the owning reference. | Append an independent byte copy. |
| `F_STORE_LOCAL` | Retain the source reference and release the old destination. | Copy bytes, release the old destination, and install the new descriptor. |
| `F_LOAD_GLOBAL` | Copy the owning reference from object state. | Object state does not store stack-owned descriptors. |
| `F_STORE_GLOBAL` | Copy the owning reference into object state. | Intern through the execution's fabric and store the resulting owning reference. |

With local string buffers enabled, stores to `string` locals use
`F_STORE_LOCAL_BUFFER` or `F_STR_FINISH_LOCAL` instead, and loads copy the
buffer reference; see [Local String Buffers](#local-string-buffers).

Stores leave the expression value on the stack. A temporary global store
therefore promotes the stored value without replacing the expression's temporary.
Local loads and stores do not implicitly intern strings.

`F_RETURN` removes the completed frame while preserving its requested return
slots, repairing temporary offsets as needed. Returned temporaries still belong
to the supplied `DataStack`. Read their bytes before popping them: `DataStack::pop()`
releases temporary storage and returns `std::monostate` for a temporary, not a
descriptor that could outlive its bytes.

## Temporary Release Costs

Releasing a temporary has three paths:

1. An empty string needs neither buffer mutation nor offset repair.
2. A range at the buffer's end is removed by truncation, with no shifting or
   stack scan.
3. A non-tail range is erased from the buffer. Trailing bytes shift, and every
   stack slot is inspected to repair offsets of nonempty temporaries after it.

The non-tail path costs up to the number of trailing bytes plus the number of
stack slots. Empty descriptors are ignored during repair. Removing a range of
stack slots with `pop_i()` can additionally shift the stack's value vector.

Stack position and buffer position are not interchangeable. Local assignment
can put newly appended bytes into an older slot. A stack-top temporary is not
necessarily at the buffer tail, and replacing a local can require compaction.

## SValue Variant Assignment Costs

Costs for assigning between the three string representations, ordered from lowest to highest cost. These are qualitative rankings: Best, Low, Moderate, Worst. Actual speed depends on string length, buffer capacity, sharing patterns, and temporary stack position. Assume local buffers enabled (the default).

| Rank | Assignment | Cost | LPC Example | Key Operation |
|------|-----------|------|---|---|
| 1 | `LpcStringRef` ← `LpcStringRef` | **Best** | `global1 = global2;` | Reference retain + release; no bytes copied |
| 2 | `LpcBufferRef` ← `LpcBufferRef` | **Low** | `string x = y;` (both locals) | Reference copy; destination buffer returned to pool if sole owner |
| 3 | `TemporaryString` ← `TemporaryString` (LHS merged) | **Low** | `func(a + b);` (expression argument) | Binary ADD fast path: left operand extended in-place, right operand popped; no byte moves |
| 3b | `TemporaryString` ← `TemporaryString` (copy, ref-released) | **Low~Moderate** (rare) | `void func(string a, string b) { b = a; }` | Push-copy creates new temporary; tail release O(1), non-tail release O(n) |
| 4 | `LpcBufferRef` ← `LpcStringRef` | **Low~Moderate** | `string local = global;` (default mode) | F_STORE_LOCAL_BUFFER: overwrite unshared buffer (Low), or allocate new (Moderate) |
| 5 | `LpcStringRef` ← `LpcBufferRef` | **Moderate** | `global_var = local_var;` | Extract bytes + intern; query table, possible allocation |
| 6 | `LpcBufferRef` ← `TemporaryString` | **Moderate** | `string local = func();` | Extract bytes + copy to buffer; release temporary (possible offset repair) |
| 7 | `TemporaryString` ← `LpcStringRef`/`LpcBufferRef` | **Moderate**[^interleaved] | `func("A"+"B"); // inside: a = val;` | Assign to temp parameter; append bytes to cat_buffer_; cleanup O(1) at tail |
| 8 | `LpcStringRef` ← `TemporaryString` | **Moderate–Worst** | `global_var = a + b;` | Extract bytes + intern + release temporary; worst-case cleanup if interleaved |

**Key principles:**
- Reference-only assignments (Cases 1–2) are cheapest: no byte copies, just reference counting
- Byte copies (Cases 3–8) scale with string length; actual cost depends on buffer reuse and temporary stack position
- Case 3 (expression arguments) is most common; temporaries only occur with expressions, not literals/variables
- By default, `CodeGenerationOptions::use_local_string_buffer = true`, compiling local assignments to `F_STORE_LOCAL_BUFFER`; see [Local String Buffers](#local-string-buffers)
- Case 4 optimization: could defer buffer allocation for read-only assignments, returning buffers to the pool

[^interleaved]: **Interleaved temporary strings** cause worst-case cleanup (O(n) offset repair). Example: `string a = function(b + "str");` creates multiple temporaries that must be released out-of-order. Best-case (O(1)): simple LIFO sequences like `string a = func(global);` where parameter and return release at tail consecutively.

The [string benchmarks](development.md#benchmarks) measure complete local
concatenation workloads with ordinary addition or the accumulator, with local
buffers enabled or disabled. They include assignment and cleanup costs, but do
not isolate assignment or release timings.

## Ordinary Concatenation

`F_BINARY_OP ADD` accepts shared and temporary string operands in any combination
and produces one stack-owned temporary. It builds a separate combined string,
copies that result into `cat_buffer_`, then removes the operands. Appending the
result before operand removal can make temporary operands non-tail ranges.

This remains the general runtime path for string addition, including dynamically
typed expressions. Other string binary operations and string/numeric addition
are unsupported. The runtime path does not require a fabric unless its result
is subsequently stored in object state.

## String Accumulator

The accumulator is a separate `std::string` plus an active/inactive flag, local
to `execute()`. It starts inactive, never aliases operand storage, and is not
accessible through stack slots or object state.

| Instruction | Required State | Effect |
|-------------|----------------|--------|
| `F_STR_BEGIN 0` | Inactive | Copy the stack-top string into the accumulator, consume it, and activate. |
| `F_STR_APPEND 0` | Active | Append the stack-top string and consume it. |
| `F_STR_FINISH 0` | Active | Push an independent temporary copy, clear the accumulator, and deactivate. |

All three instructions require a zero argument. Begin and append accept shared
or temporary strings and reject null references, non-string values, and missing
operands within the current frame. The accumulated length must fit in `uint32_t`;
finish also enforces the stack buffer's total-size limit.

Validation or allocation failure leaves that instruction's operand and logical
accumulator contents unchanged. Failure of an execution destroys its accumulator;
it does not roll back earlier stack operations or stores.

Calls and returns, including implicit returns at bytecode end, require an inactive
accumulator. There is no save/restore across calls. Finish the current chain before
starting another or making a call. Ordinary instructions can run while it is
active, provided bytecode respects these constraints.

Capacity is reused across chains within the same invocation, but not across
`execute()` invocations. Appends avoid intermediate stack-buffer results and
repeated prefix copies except when accumulator growth requires reallocation.
Consuming a shared operand or buffer-tail temporary needs no buffer compaction
or offset scan; a non-tail temporary still uses normal cleanup. `F_STR_FINISH`
still copies the complete result, and a later local store still has normal
replacement costs; `F_STR_FINISH_LOCAL` avoids both by moving the result into a
local buffer.

## Local String Buffers

A local string buffer is an `LpcBufferRef`: a reference-counted `LpcBuffer`
holding a mutable `std::string`, acquired from the execution fabric's buffer pool
(see [vm-fabric.md](vm-fabric.md)). Unlike a temporary, it owns its bytes
independently of the `DataStack`, so replacing a local never compacts the stack
buffer, and its capacity is reused across assignments and, through the pool,
across executions. Copying an `LpcBufferRef` shares the same buffer; it does not
copy the bytes. On a write, the VM reuses the destination buffer only when that
local holds its sole reference. If another value still refers to it, the VM
writes the new value to a different buffer, leaving the other reference's bytes
unchanged. This copy-on-write behavior preserves LPC string value semantics.

`CodeGenerationOptions::use_local_string_buffer` (default `true`) lowers stores
to scalar `string` locals to `F_STORE_LOCAL_BUFFER` instead of `F_STORE_LOCAL`.
Executions of such code require a fabric.
The handler copies the stack-top string into an `LpcBuffer` from the execution's
fabric pool. If the local already holds an unshared buffer, its
bytes are overwritten in place, reusing capacity. Otherwise a new buffer is
acquired, so loaded copies of the old reference are unchanged (copy-on-write).
Non-string values are stored as by `F_STORE_LOCAL`. Loads copy the buffer
reference. Buffers are accepted wherever string operands are, and
`F_STORE_GLOBAL` interns their bytes.

When the accumulator is also enabled, `local = <eligible chain>` and
`local += <eligible terms>` end with `F_STR_FINISH_LOCAL slot` instead of
`F_STR_FINISH` and a store. It swaps the accumulator's storage with the local's
unshared buffer (or a newly acquired one), so the result is never copied, then
pushes the local's reference as the expression value. The accumulator inherits
the buffer's previous capacity.

## Compiler Lowering

Accumulator lowering is enabled by default in both `LpcCompiler::generate_code()`
and `lpcvm::Bytecode`. Eligible `+` trees contain only string literals and declared
scalar string variables. The parser preserves variable declaration types so
the emitter can distinguish strings from `mixed`, numeric, and array values.

The emitter flattens eligible trees, including parenthesized chains, in operand
order. For `left + right + suffix` using local slots 0, 1, and 2, it emits:

```text
F_LOAD_LOCAL 0
F_STR_BEGIN 0
F_LOAD_LOCAL 1
F_STR_APPEND 0
F_LOAD_LOCAL 2
F_STR_APPEND 0
F_STR_FINISH 0
```

Two-term additions use the same path with one append. Numeric and `mixed`
expressions, calls, and side-effecting expressions retain ordinary lowering;
eligible subtrees can still be optimized independently. The compiler does not
evaluate calls or another accumulator chain while an accumulator is active.
Compound `+=` on a global, or on a local with buffers disabled, uses binary
addition followed by a store, although an eligible chain on its right-hand side
can use the accumulator. With buffers enabled, `+=` and `=` on string locals
finish into the local's buffer as described above.

Setting `use_string_accumulator` or `use_local_string_buffer` to `false` in the
`CodeGenerationOptions` passed to `generate_code()` or the `Bytecode`
constructor selects the alternative lowering for comparison. The string
benchmarks compile identical LPC source in all four combinations, verify the
selected bytecode, and time execution rather than compilation.

## Ownership and Verification

Stacks, the accumulator, mutable object state, and reference counts are
thread-confined. String views borrow their storage; retain an owning shared
reference or copy bytes when a longer lifetime is needed.

Implementation lives in [lpc_string.cpp](../src/lpc_string.cpp),
[vm_executor.hpp](../src/vm_executor.hpp),
[vm_executor.cpp](../src/vm_executor.cpp), and
[code_generator.cpp](../src/code_generator.cpp). Regression coverage includes
[test_vm_data_stack.cpp](../tests/test_vm_data_stack.cpp),
[test_vm_string_operator.cpp](../tests/test_vm_string_operator.cpp), and
[test_lpc_compiler.cpp](../tests/test_lpc_compiler.cpp).