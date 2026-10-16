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
- Everything in the virtual machine is an **object** registered with a name.
  - The object **loader** can load bytecodes by reading from a bytecode file or compiling LPC source files on-demand.
  - Loaded bytecode is reference counted. The initiall reference is added by a **blueprint object**, which is registered with a unique name, e.g. `"/std/player"`.
  - More objects can be created by **cloning** from the blueprint object, each with a name derrived from the blueprint object, e.g. `"/std/player#24`. Cloned objects add references to the loaded bytecode.
  - Each object, blueprint or cloned, has its own states (**variables**).
  - **Hot-reloading** is achieved by *replacing* the registered blueprint object with a newly loaded one. Existing cloned objects are not affected by hot-reloading.
- The stack-machine manages memory with the object registry and **evaluation stack**.
  - A variable can possess a `svalue` (filling one stack element): the value of a primitive data type, a strong reference to a compound data type, or a weak reference to an object. There is no pointer semantic in the LPC language.
    - Primitive data types (`int`, `float`, `string`, ... etc.) are passed by value.
    - Compound data types (`function`, `mapping`, array of any types) have reference-counted lifecycle, and are passed by reference.
    - An `object` is passed by reference, and must be **destructed** explicitly (making all references to it evaluated as integer zero or `false`).
  - Variables in objects are released when the object is destructed; varables in the evaluation stack (e.g. function arguments and local variables) are released when they are poped from the stack. Compound data types are released when their reference count reaches zero.

## LPC Language
> [!NOTE]
> LPC language was designed decades ago and lacks a published standard.
> The LPMud implementation also suffer from out-dated architecture and non-reentrant program structures.

LPCVM starts from a minimal set of [LPC grammar](src/lpc_parser.yy) with basic programmings and leave fancy features for efuns.
The goal is to proivde an east-to-use LPC language that LPMud wizards are already familiar with.
Some enhancements and simplifications are added to the LPC language with the help of modern Bison/Flex/C++ capabilities.

### LPC frontend CLI
- A command-line tool for LPC parsing frontend.

### C++ Lexer and Parser
- Reentrant lexer and parser with `yyscan_t`.
- Reentrant LPC compiler with `LpcCompiler::Context`.

### Modern Memory Management
- Standard C++ `<memry>` and `<utility>`.
- Robust error handling with `std::runtime_error` and unwinding.
