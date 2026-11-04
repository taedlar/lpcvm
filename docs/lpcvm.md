# LPCVM Design

See also [vm-architecture.md](vm-architecture.md) for the stack machine, bytecode
and sandbox architecture.

## Objects

- Objects are defined with LPC language. Each LPC source file (`*.lpc`) defines an **object class** with method functions and variables.
- The LPC compiler compiles source files to object class images (function bytecodes, constant pool, and virtual table of method functions).
- The object loader **loads** an object by invoking the LPC compiler to compile LPC source files in-memory.
  - The object class image is reference-counted, with the first reference held by the loaded object.
  - The loaded object is registered with a unique name (e.g. `"std/player"`) and serves as a **blueprint object**.
  - More objects can be **cloned** from blueprint objects, adding more references to their object class image.
- Hot-reloading replaces the registered blueprint with a newer version. Existing clones continue to use the old version.
- Each object has its own **object state** (variables, I/O buffers, and other attributes).
  - The LPC `object` data type is a handle to an individual object and allows access to its state.
  - The LPC `call_other` efun calls methods defined in the object class image referenced by the object.
  - The virtual machine can also invoke methods on I/O events, timers, or policy contracts.
  - An LPC object must be **destructed** explicitly once loaded, to emulate physical objects (e.g. monsters) in a virtual world.

## Method Functions

Methods share their bytecode and constants through an `ObjectClass`; object
variables belong to the target object's `ObjectState`. Each method binding records
its signature and bytecode range. `ObjectClass::lookup_method()` resolves a name
to an `Execution` with the class context. Its optional third and fourth arguments
attach an `ObjectState*` and a fabric pointer (either a `ManagedRuntime*` or `Sandbox*`), respectively.

Calls within a class use method-table indices. The caller pushes the method index
and arguments, and the executor saves the caller's frame on the return stack.
Arguments, locals, and temporary values occupy the data stack. Returning removes
the callee's frame while leaving its return values for the caller.

## Variables

A variable holds an `SValue`: a value that fits in one stack element, such as a primitive value, a reference to a compound value, or an object handle.

- Numeric types (`int`, `float`, ...) are stored in the `SValue` and passed by value.
- Strings use owning `LpcStringRef` values pointing to immutable shared bytes.
- Compound types (`function`, `mapping`, arrays) live in heap memory with reference-counted lifecycle and are passed by reference.
- Objects live in heap memory and are passed by handle (weak reference).
- There is no raw pointer semantics in LPC.
- Object variables are destroyed with the object, stack variables (arguments and locals) when popped, and compound values when their reference count reaches zero.

## LPC Language and Compiler

> [!NOTE]
> LPC was designed decades ago and lacks a published standard. LPMud implementations also
> suffer from out-dated architecture and non-reentrant program structures.

LPCVM starts from a minimal set of [LPC grammar](../src/lpc_parser.yy) and leaves fancy features to efuns.
The goal is an easy-to-use LPC that LPMud wizards are already familiar with, with some
enhancements and simplifications.

The compiler consists of:

- `lpcfront`, a command-line frontend that dumps the AST.
- A reentrant preprocessor, lexer and parser (`flex` and `bison`) that output AST variants.
- A visitor-pattern bytecode generator that emits bytecode from AST nodes.

Supported language features:

- Expressions: arithmetic, bitwise, comparison, unary (`-`, `~`, `!`), assignment (`=` and compound forms such as `+=` and `<<=`), `++`/`--` (prefix and suffix), ternary `?:`, and short-circuit `&&`/`||`. The result of `&&`/`||` is the last operand evaluated, not normalized to 0/1.
- Statements: blocks with local variables, `if`/`else`, `while`, `do`/`while`, `for`, `break`, `continue`, and `return`.
- Binary operations on mixed numeric types are promoted to a common type: floating point wins, and mixed-sign integers become `int64` (or `uint64` when the unsigned operand is 64-bit).

## Memory Management

- Standard C++ containers and smart pointers for internal data structures.
- A 16-byte `SValue` (`std::variant`) that stores LPC int, float, double, and references.
- `lpcvm::SharedPtr` for LPC mapping and array reference semantics: strong reference with a prefix control block, faster and smaller than `std::shared_ptr`.
- `lpcvm::Handle` for LPC object reference semantics: weak reference with a prefix control block, faster and smaller than `std::weak_ptr`.
- Errors are reported with `std::runtime_error` and stack unwinding.

## Benchmarks

Benchmarks need `BUILD_TESTING=ON`. Use an optimized `Release` configuration for comparisons.

### Bytecode

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --target bench_bytecode
./build/benchmarks/bench_bytecode --benchmark_min_time=0.1s --benchmark_repetitions=5
./build/benchmarks/bench_bytecode --benchmark_filter=BM_Dispatch --benchmark_min_time=0.1s
~~~

- `BM_ReturnOnly` measures the execution entry/return baseline.
- `BM_Dispatch` varies stream size, zero to three extended-argument prefixes,
  and starting offsets from a known 64-byte-aligned address. Zero-valued prefixes
  keep the workload identical while isolating decoding overhead; they are
  intentionally redundant and are not emitted by the compiler.
- `BM_IntegerAdd` measures constant loads and integer addition.
- `BM_LocalAccess` measures repeated local loads, stores, and stack pops.

Bytecode construction, constant-pool setup, and a validating stack warm-up are
outside the timed loop. Arithmetic and local-access timings include popping one
returned value per execution so stack capacity can be reused. These are repeated
execution benchmarks, not cold-cache or isolated memory-bandwidth measurements.

`items_per_second` counts decoded two-byte words, including extended-argument
prefixes. `logical_ops/s` excludes prefixes, and `bytes_per_second` reports
bytecode bytes traversed, not physical memory traffic.

### Macro expansion

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build --target bench_macro_expansion
./build/benchmarks/bench_macro_expansion --benchmark_min_time=0.1s --benchmark_repetitions=5
./build/benchmarks/bench_macro_expansion --benchmark_filter=BM_ObjectMacroChain
~~~

The suite covers plain-text scanning, object-like macros, function arguments,
nested function invocations, object-macro chains, and recursion guards. Workloads
vary the number of input lines and, for nested expansion, the depth.

Source construction, macro definition parsing, and an exact-output validation
are outside the timed loop. Each iteration calls `LpcCompiler::preprocess` on
the same input with prepared definitions; timings include scanner and compiler
context setup, input copying, expansion, and output allocation and destruction.
No LPC parsing or code generation is performed. `items_per_second` counts input
lines (not recursive macro expansions), and `bytes_per_second` counts input bytes.

## Classes

The `lpcvm` namespace provides bytecode generation, runtime diagnostics, object
management, and reference-counted ownership through the following types:

- [lpcvm::Bytecode](#class-lpcvmbytecode)
- [lpcvm::DataStack](#class-lpcvmdatastack)
- [lpcvm::Exception](#class-lpcvmexception)
- [lpcvm::Execution](#struct-lpcvmexecution)
- [lpcvm::Handle](#class-lpcvmhandle)
- [lpcvm::ReturnStack](#class-lpcvmreturnstack)
- [lpcvm::Sandbox](#class-lpcvmsandbox)
- [lpcvm::SharedPtr](#class-lpcvmsharedptr)

### class `lpcvm::Bytecode`

Declared in [code_generator.hpp](../src/code_generator.hpp).

Builds VM bytecode in a caller-owned byte vector. The LPC compiler uses it to
translate AST expressions, statements, and function bodies into stack-machine
instructions. It handles extended instruction arguments and can record source
locations in an `ObjectClass` for runtime diagnostics.

### class `lpcvm::DataStack`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Stores arguments, locals, and expression operands as a contiguous vector of
`SValue`s. The executor addresses frame slots by index so stack growth does not
invalidate the frame pointer. Push and pop operations manage value lifetimes;
invalid indices and stack underflow raise exceptions. The caller owns the stack
and can reuse its capacity across executions.

### class `lpcvm::Exception`

Declared in [vm_exception.hpp](../src/vm_exception.hpp).

`Exception<E>` adds LPC source context to a standard C++ exception while remaining
catchable as `E`. It uses the program and instruction address to identify the
source file, function, line, and column, and includes that context in `what()`.
When source information is unavailable, it preserves the original error message.

### struct `lpcvm::Execution`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Holds a bytecode range and non-owning pointers to its class, optional object
state, and optional fabric, which must remain alive during execution.
`execute()` runs synchronously
with caller-supplied data and return stacks. Arguments are already on the data
stack; normal return removes them and the locals, leaves the results, and reports
the number of return values. Object-variable access requires an object state.
String literals emit `F_CONST_STR`, which interns the constant-pool bytes through
`make_shared_string()` in the supplied fabric and pushes an owning reference.
Executing this opcode without a fabric throws. Returned strings and strings
stored in object state remain valid after the execution, program, or fabric is
destroyed, as long as their owning references remain alive.

```cpp
ManagedRuntime fabric; // A Sandbox can also be supplied.
lpcvm::Execution entry;
if (program->lookup_method("set_message", &entry, &state, &fabric))
    entry.execute(control, stack);
```

The program counter and frame pointer are local to the call; this type does not
store a suspended continuation.

### class `lpcvm::Handle`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`Handle<T, RefType>` observes a reference-counted object without keeping it alive.
LPC object values use handles so that passing an object around does not extend
its lifetime. `expired()` checks whether the object still exists; `lock()` returns
an optional strong reference for access. The shared control block remains valid
until all strong and weak references are released.

### class `lpcvm::ReturnStack`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Stores caller frames for nested method calls. Each frame saves the return address,
data-stack frame index, and caller's bytecode bounds. Returning restores this
context and pops the frame. `execute()` records the initial return-stack depth so
normal completion preserves any frames supplied by its caller. Return values
remain on the data stack.

### class `ManagedRuntime`

Declared in [managed_runtime.hpp](../src/managed_runtime.hpp).

Provides default file access and shared-string services without requiring a
sandbox or object registry. This is the base fabric for numeric-only execution.
`make_shared_string()`, `shared_string_allocation_size()`, and `sweep_shared_strings()` are virtual.
The free factory `lpcvm::make_shared_string(fabric, value)` forwards to the
virtual member. Custom storage implementations should override all three
services consistently. Numeric-only execution can omit the fabric.

The default fabric interns immutable shared strings. Equal byte sequences created
through `lpcvm::make_shared_string(fabric, value)` share one `LpcString` within
that fabric. The factory copies the input, including embedded NUL bytes.
The table holds a strong reference until sweeping or fabric destruction;
external owning references remain valid after the fabric is destroyed.

```cpp
ManagedRuntime fabric;
auto text = lpcvm::make_shared_string(fabric, "hello");
auto bytes = fabric.shared_string_allocation_size(); // 22 on the 64-bit build
text = {};
auto removed_bytes = fabric.sweep_shared_strings(); // 22
```

`shared_string_allocation_size()` sums the allocation sizes of distinct strings
currently in the table: reference-count block, string metadata, characters, and
one trailing NUL byte (17 + length on the 64-bit build). It excludes allocator
and hash-table overhead. `sweep_shared_strings()` removes entries whose only
strong reference belongs to the table and returns their total allocation size.
Both operations take linear time in the table size and run on the owning thread.
Sweeping retains hash buckets; outstanding weak handles can delay the physical
release of removed strings' allocations.

### class `lpcvm::Sandbox`

Declared in [vm_sandbox.hpp](../src/vm_sandbox.hpp).

A concrete fabric implementation that manages a VM's named LPC objects. It loads and compiles
source files into blueprint objects, finds registered objects, and creates clones that share
a compiled `ObjectClass` but have separate state. The object table holds strong references;
callers receive weak handles. Explicit destruction removes the table's reference,
allowing the object to expire once any remaining strong references are released.

Shared-string allocation, accounting, and sweeping are inherited from
`ManagedRuntime` and remain available through the sandbox.

The constructor accepts a mudlib root for the intended resource boundary. The
current loader opens the supplied path directly; it does not yet enforce
filesystem confinement to that root.

### class `lpcvm::SharedPtr`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`SharedPtr<T>` owns a heap-allocated value through reference counting. Copies
share ownership, and releasing the last strong reference destroys the value.
It provides reference semantics for LPC arrays and mappings, and ownership of
registered LPC objects. The object and its prefix control block share one
allocation.

`SharedPtr` and `Handle` use non-atomic reference counts. All references to the
same object must stay on a single VM worker thread.

### function `lpcvm::create_shared`

`create_shared<T>(args...)` allocates an object with its control block, forwards
arguments to its constructor, and returns its initial `SharedPtr<T>`. Use this
factory to create values managed by LPCVM's reference-counted pointers.
