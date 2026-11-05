# LPCVM Development

This document covers the LPC language, compiler and preprocessor, and benchmark
workflows. Run the commands below from the repository root.

See [lpcvm.md](lpcvm.md) for the object and value model and runtime class reference,
and [vm-architecture.md](vm-architecture.md) for the stack machine, bytecode, and
sandbox architecture.

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
