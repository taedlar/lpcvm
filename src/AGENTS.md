# Source Guidelines

## Design References

- Before adding classes or structs, check [lpcvm.md](../docs/lpcvm.md) for the object model and runtime types.
- Use [vm-architecture.md](../docs/vm-architecture.md) for stack execution and fabric boundaries, and [lpc-language.md](../docs/lpc-language.md) for the evolving language and compiler.
- Follow [development.md](../docs/development.md) for builds, unit tests, and benchmarks. Rebuild before running relevant tests for source changes.

## Naming and Source Layout

- Use `lpc_` for LPC language components and `vm_` for VM components; filenames alone do not determine API visibility.
- Use `lpcvm` for new VM runtime components and `Lpc` for LPC abstraction classes. Existing runtime types also live in the global namespace; follow surrounding code without unrelated namespace migrations.
- Use PascalCase for class and struct names. Follow local conventions for members and functions.
- Edit Flex (`*.l`) and Bison (`*.yy`) sources, not their generated C++ files.

## Compiler

- Keep source-language rules in the compiler and lower them to the VM's bytecode semantics.
- `LpcCompiler` accepts a caller-supplied `FILE*` or source string. `preprocess()` returns text; `parse()` returns success and exposes the AST through `get_ast()`; `generate_code()` returns a `std::shared_ptr<ObjectClass>`.
- `#include` processing and compiler inheritance integration are currently TODOs. When implementing source loading, route filesystem access through the supplied fabric.
- Treat source text as untrusted. Route sensitive operations through the fabric; `ManagedRuntime` alone does not enforce sandbox confinement.

## Bytecode and Runtime

- Keep bytecode execution language-neutral. High-level policies, including possible LPC master-object hooks, belong at the fabric boundary.
- Define opcodes in `vm_opcodes.hpp` and handlers in `vm_executor.cpp`; keep emission, decoding, stack effects, and tests consistent.
- Store literal constants in the constant pool and load them by index. Instruction arguments also encode slot indices, counts, operation selectors, and branch offsets.
- Address arguments and locals relative to the frame pointer in `DataStack`; address instance variables relative to the state offset in `ObjectState`.
- Keep program bytecode read-only during execution and mutable state separate. Preserve call-frame cleanup and return-value semantics.
- Treat bytecode as untrusted: validate operands, stack access, and control-flow bounds when adding handlers. Existing checks are not a complete bytecode verifier.
- Preserve thread confinement of stacks, mutable state, and reference counts. Route resource access through the fabric.
