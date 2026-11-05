# LPCVM Development

This document covers building, unit tests, and benchmark workflows. Run the
commands below from the repository root.

See [lpcvm.md](lpcvm.md) for the object and value model and runtime class reference,
and [vm-architecture.md](vm-architecture.md) for the stack machine, bytecode, and
sandbox architecture. See [lpc-language.md](lpc-language.md) for the language
and compiler outline currently under development.

## Build

Install a C++ compiler, CMake 3.28 or newer, Bison 3.2 or newer, Flex 2.5 or
newer, Git, and a build tool such as Make or Ninja. CMake fetches the third-party
dependencies from GitHub during configuration, so the first configure requires
network access.

Configure a debug build with tests enabled, then compile:

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build --parallel
~~~

This builds the VM library, `lpcfront`, unit tests, and benchmarks. To omit tests
and benchmarks, configure with `-DBUILD_TESTING=OFF`. After editing source files,
rerun the build command to compile the changes.

## Unit Tests

After building with `BUILD_TESTING=ON`, run the test suite through CTest:

~~~bash
ctest --test-dir build --output-on-failure
~~~

CTest runs the GoogleTest unit tests and the `lpcfront` command-line tests.
`--output-on-failure` displays diagnostics for failed tests. To list available
tests or run only the constant-pool tests:

~~~bash
ctest --test-dir build --show-only
ctest --test-dir build --output-on-failure -R '^ConstTableTest\.'
~~~

Rebuild before rerunning tests; CTest does not compile source changes. For a
multi-configuration generator such as Visual Studio, add `--config Debug` to the
build command and `-C Debug` to the CTest commands.

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
