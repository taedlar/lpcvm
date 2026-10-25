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
LPCVM intended to develop a compact virtual stack-machine that can be safely sandboxed.
A LPC mudlib is a sandbox that is restricted to see only a portion of the filesystem.

### Objects
- Objects are defined with LPC language. Each LPC source file (`*.lpc`) defines an object's variables and methods.
- The sandbox's object **loader** can load bytecodes by reading from a bytecode file or compiling LPC source files on-demand.
- Loaded bytecode is reference counted. The initiall reference is added by a **blueprint object**, which is registered with a unique name, e.g. `"/std/player"`.
- More objects can be created by **cloning** from the blueprint object, each with a name derrived from the blueprint object, e.g. `"/std/player#24`. Cloned objects add references to the loaded bytecode.
- Each object, blueprint or cloned, has its own instance of states (**variables**, I/O buffers, and other attributes).
- Hot-reloading is achieved by *replacing* the registered blueprint object with a newly loaded one. Existing cloned objects are not affected by hot-reloading.

### Variables
- A variable can hold a `SValue` (a value fits in one stack element), e.g. the value of a primitive data type, a reference to a compound data type, or a handle of an object.
  - Primitive data types (`int`, `float`, `string`, ... etc.) are stored in SValue and passed by value.
  - Compound data types (`function`, `mapping`, array of any types) are stored in heap allocated memory with reference-counted lifecycle and passed by reference.
  - Objects are stored in heap allocated memory and passed by handle (weak reference). Objects must be **destructed** explicitly.
  - There is no raw pointer semantics in LPC language.
- Variables in objects are destroyed when the object is destructed.
- Variables in the operand stack (e.g. function arguments and local variables) are destroyed when poped from the stack.
- Heap allocated compound data types are destroyed when their reference count reaches zero.

### Stack Machine
- LPCVM implements a simple stack machine with a small set of bytecode instructions:
  - Each instruction consists of 8 bits operation code and 8 bits argument.
  - If an instruction needs an argument wider than one byte (e.g. jump distance), it can be extended to at most 32 bits by prefix extended argument instruction.
- The bytecode executor context consists of:
  - Start and end of bytecode (immutable)
  - Constant pool (immutable)
  - Global variables
  - Evaluation stack
- SValues in global variables and operand stack are designed for thread-confinement architecture. No synchronization on access.
- No global program counter or stack pointers.

## LPC Language
> [!NOTE]
> LPC language was designed decades ago and lacks a published standard.
> The LPMud implementation also suffer from out-dated architecture and non-reentrant program structures.

LPCVM starts from a minimal viable set of [LPC grammar](src/lpc_parser.yy) and leaves fancy features for efuns.
The goal is to proivde an east-to-use LPC language that LPMud wizards are already familiar with.
Some enhancements and simplifications are added to the LPC language with the help of modern Bison/Flex/C++ capabilities.

### LPC compiler CLI
- A command-line LPC compiler `lpcfront`.

### C++ Lexer and Parser
- Reentrant lexer and parser.
- Reentrant LPC compiler.

### Modern Memory Management
- Standard C++ `<memry>` for internal data structures.
- 16-bytes SValue using `std::variant` that stores LPC int, float, double, references (8 bytes index + 8 bytes value)
- `LpcSharedPtr` for LPC mapping, array reference semantic:
  - Strong reference (prefix control blcok)
  - Faster and smaller than `std::shared_ptr`
- `LpcHandle` for LPC object reference semantic:
  - Weak reference (prefix control block)
  - Faster and smaller than `std::weak_ptr`
- Robust error handling with `std::runtime_error` and unwinding.
