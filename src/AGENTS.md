# Coding Agent Instructions

## Source files naming conventions

- Prefix `lpc_` means components for the LPC language features.
- Prefix `vm_` means components for the virtual machine's public C++ API or definitions.
- Other files are for internal implementations.

## C++ class/struct naming conventions

- For virtual machine runtime public components, use the `lpcvm` namespace with proper hierarchy.
- For LPC components, use `Lpc` prefix for the abstraction (wrapper) class.
  Put the actual VM implementaion of LPC components in `lpcvm` namespace.
  See `lpc_types.hpp` for example.
- For internal classes, use regular camel case style.

## LPC compiler

- **Inputs**:
  1. C++ caller supplied `FILE` pointer or source string.
  2. LPC code requested by `#include` preprocessor directive or `inherit` clause. (must be confined with `ManagedRuntime`)
- **Outputs**:
  - Preprocessor `preprocess()` produces preprocessed source string.
  - Parser `parse()` produces the abstract syntax tree (AST node).
  - Code generator `generate_code()` produces the `ObjectClass` instance that holds the compiled bytecode and data.
- The compiled bytecode is read-only in virtual machine runtime. See `vm_executor.hpp` for the execution context.

## Virtual machine opcodes

- See `vm_opcodes.hpp` for list of virtual machine opcodes.
- See `vm_executor.hpp` and `vm_executor.cpp` for opcode operations.
- Constants used by the bytecode must be interned in the constant pool. Opcodes can load constants using a index operand.
- Variables are stored in the `DataStack` or `ObjectState` instances.
  - Function arguments and local variables has a static index, based on the frame pointer in data stack.
  - Global variables has a static index, based on the global offset in object state.
