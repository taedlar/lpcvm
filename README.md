# LPCVM
A sandboxed virtual machine inspired by LPMud's **blueprints and clones** architecture.

This is NOT a LPMud fork (not using LPMud source codes). LPCVM is intended to be released with a permissive MIT license.

The blueprint-based OOP concepts and many other terms were borrowed from LPMud.
An LPC-like (ANSI C like) programming language is developed for the sandboxed virtual machine and made minimal deliberately.

## How To Build
From the top source directory:
~~~bash
cmake -S . -B build
cmake --build build
~~~

## Sandboxed Virtual Machine
LPCVM intended to develop a compact virtual stack-machine that can be safely sandboxed:
- Everything is **object**, each with one shared blueprint (a LPC source file in the mudlib directory) and clone instances (variables).
- The object loader can *load* persisted blueprint bytecodes or **compile** LPC source code into bytecode on-demand.
- Besides object loader, the virtual machine can **clone** an object from blueprint via the `new` operator, which allocates a copy of all the global/static variables declared in the blueprint.
- A **variable** can store a primitive data type or a reference to a compound data type.
- All variables must be either on the virtual machine's **value stack** or referenced by an object:
  - Primitive data types (`int`, `float`, `string`, ... etc.) are passed by value.
  - Compound data types (`object`, `array`, `mapping`, `function`, ... etc.) have **reference counted** lifecycle, and are passed by reference.
  - Blueprints are also reference counted by object clones and **inheritance** from another blueprints.
- Function calls in the virtual stack-machine is implemented with a value stack and a **control stack**.
  - Native multithreading for LPC is possible by isolated (program counter, value stack, control stack). Synchronization for referenced entities and I/O are also required.
  - Native RPC support for the virtual machine (via **proxy** node and serialization/deserialization) can enable creation of a distributed virtual machine.
- The virtual machine calls **apply functions** (methods) in objects upon various events.
- Sandboxing is enforced at the **efuns** boundary, which is the only way LPC code can interact with outside world.

The sandboxed virtual machine (object loader and sandbox runner) is provided as a statically linked library with APIs.

### Blueprints
- Bytecodes of functions (methods)
- Constant pool
- Inheritance table
- Symbol table

### Clones
- Reference of blueprint
- Variables

## LPC Language
> [NOTE!]
> LPC language was designed decades ago and lacks a published standard.
> The LPMud implementation also suffer from out-dated architecture and non-reentrant program structures.

LPCVM starts from a minimal set of [LPC grammar](src/lpc_parser.yy) with basic programmings and leave fancy features for efuns.
The goal is to proivde an east-to-use LPC language that LPMud wizards are already familiar with.
Some enhancements and simplifications are added to the LPC language with the help of modern Bison/Flex/C++ capabilities.

### C++ Lexer and Parser
- Reentrant lexer and parser with `yyscan_t`.
- Reentrant LPC compiler with `LpcCompiler::Context`.

### Modern Memory Management
- Standard C++ `<memry>` and `<utility>`.
- Robust error handling with `std::runtime_error` and unwinding.
