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
- Everything is **object**, each with a shared blueprint (mapped to a LPC source file in the mudlib directory) and a clone of variables.
- The object loader **loads** persisted blueprint bytecodes (opcodes) or **compiles** LPC code into blueprint bytecodes on-demand.
- When cloning an object (i.e. `new` operator) from a loaded blueprint, allocate a copy of all the global variables and share (read-only) the same blueprint bytecodes.
- All variables must be either on the value stack or be an object's variable (nested):
  - There are **primitive** data types (`int`, `float`, `string`, ... etc.) and compund data types. Primitive data types are passed by value.
  - Objects and compound data types (`array`, `mapping`, `function`, ... etc.) have **reference counted** lifecycle, and are passed by reference.
- The virtual machine calls **apply** functions in objects upon various events.
- Sandboxing is enforced at the **efun** boundary, which is the only way LPC code can interact with outside world.

The sandboxed virtual machine (object loader and sandbox runner) is provided as a statically linked library with APIs.

### LPC Language
LPC language was designed decades ago and lacks a published standard.
LPCVM starts from a minimal set of [LPC grammar](src/lpc_grammar.y) with basic programmings and leave fancy features for efuns.

## Tools
- LPC linter
- LPC bytecode compiler
