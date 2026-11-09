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
./build/benchmarks/bench_bytecode --benchmark_filter=BM_StringLocalConcat --benchmark_min_time=0.1s --benchmark_repetitions=5
~~~

- `BM_ReturnOnly` measures the execution entry/return baseline.
- `BM_Dispatch` varies stream size, zero to three extended-argument prefixes,
  and starting offsets from a known 64-byte-aligned address. Zero-valued prefixes
  keep the workload identical while isolating decoding overhead; they are
  intentionally redundant and are not emitted by the compiler.
- `BM_IntegerAdd` measures constant loads and integer addition.
- `BM_LocalAccess` measures repeated local loads, stores, and stack pops.
- `BM_StringLocalConcat` and `BM_StringLocalConcatAccumulator` compare ordinary
    stack addition against the explicit string accumulator for the same local
    assignment: `result = left + right + ...`. Both vary assignment count, each
    operand's byte length, and chain length (2, 8, or 32 terms). Operands remain
    fixed-size shared strings; each completed temporary replaces the previous
    result local, so results do not grow across assignments. Timings include local
    loads, concatenation, local stores, temporary-string cleanup, and returning
    and popping the final result. Accumulator timings also include begin/finish
    instructions and copying the completed result into the stack buffer. Its
    capacity is reused within each execution, but not across executions.
    `concats/s` counts binary concatenations (assignments times terms minus one).
    Compare elapsed time or `concats/s`, not instruction throughput: the two paths
    execute different instruction counts. Both use the same tail-release shortcut.

Both string benchmarks parse identical LPC source and generate the complete
function through `LpcCompiler::generate_code(false)` or `generate_code(true)`.
They verify that the emitted bytecode uses the requested mode. Parsing and code
generation are outside the timed loop; only repeated VM execution is measured.
Assignments are unrolled, so the comparison does not include LPC loop overhead.
To save a Release comparison with interleaved repetitions:

~~~bash
./build/benchmarks/bench_bytecode --benchmark_filter=BM_StringLocalConcat \
    --benchmark_min_time=0.05s --benchmark_repetitions=5 \
    --benchmark_enable_random_interleaving=true \
    --benchmark_out=build/string_concat_release.json --benchmark_out_format=json
~~~

Compare median CPU time for matching parameters, dividing by `assignments` for
time per assignment. Every assignment includes `terms - 1` binary concatenations.

Bytecode construction, constant-pool setup, and a validating stack warm-up are
outside the timed loop. Arithmetic, local-access, and string timings include
popping one returned value per execution so stack capacity can be reused. These
are repeated execution benchmarks, not cold-cache or isolated memory-bandwidth
measurements.

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
