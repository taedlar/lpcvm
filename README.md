# LPCVM
A sandboxed virtual machine inspired by LPMud's **blueprints and clones** architecture.

This is NOT a LPMud fork (it does not use LPMud source code). LPCVM is intended to be released with a permissive MIT license.
Blueprint-based OOP concepts and many other terms are borrowed from LPMud, and a deliberately minimal LPC-like (ANSI C like) language is developed for the VM.

## How To Build
From the top source directory:
~~~bash
cmake -S . -B build
cmake --build build
~~~

With `BUILD_TESTING=ON`, the unit tests and benchmarks are also built (see [Benchmarks](docs/development.md#benchmarks)).

## Overview
- **Sandboxed stack machine**: a compact bytecode VM whose mudlib sees only a portion of the filesystem. See [vm-architecture.md](docs/vm-architecture.md).
- **Objects**: each `*.lpc` file defines an object class. Loaded **blueprints** are cloned into objects with separate state, and blueprints can be hot-reloaded. Objects are destructed explicitly.
- **Values**: 16-byte `SValue` slots; primitives by value, compound types by reference-counted pointer, objects by weak handle. No raw pointers.
- **Thread-confined**: stacks, object state and reference counts need no synchronization; there is no global program counter or stack pointer.

## LPC Language
LPCVM starts from a minimal set of [LPC grammar](src/lpc_parser.yy) and leaves fancy features to efuns, aiming to stay familiar to LPMud wizards.

- Expressions: arithmetic, bitwise, comparison, unary, assignment (including compound forms), `++`/`--`, `?:`, and short-circuit `&&`/`||`.
- Statements: blocks, `if`/`else`, `while`, `do`/`while`, `for`, `break`, `continue`, and `return`.
- Tooling: `lpcfront` command-line frontend (dumps the AST), reentrant `flex`/`bison` preprocessor, lexer and parser, and a bytecode generator.

## Documentation
- [docs/lpcvm.md](docs/lpcvm.md): ontology of objects, methods, values, ownership, and runtime classes.
- [docs/vm-fabric.md](docs/vm-fabric.md): fabric implementations, runtime services, and C++ usage.
- [docs/development.md](docs/development.md): build, unit-test, and benchmark workflows.
- [docs/lpc-language.md](docs/lpc-language.md): language grammar and compiler outline (work in progress).
- [docs/vm-architecture.md](docs/vm-architecture.md): executor, bytecode instruction set, and sandbox architecture.
