# LPCVM Design

The `lpcvm` namespace provides bytecode generation, runtime diagnostics, object
management, and reference-counted ownership through the following types:

- [lpcvm::Bytecode](#class-lpcvmbytecode)
- [lpcvm::Exception](#class-lpcvmexception)
- [lpcvm::Handle](#class-lpcvmhandle)
- [lpcvm::Sandbox](#class-lpcvmsandbox)
- [lpcvm::SharedPtr](#class-lpcvmsharedptr)

## class `lpcvm::Bytecode`

Declared in [code_generator.hpp](../src/code_generator.hpp).

Builds VM bytecode in a caller-owned byte vector. The LPC compiler uses it to
translate AST expressions, statements, and function bodies into stack-machine
instructions. It handles extended instruction arguments and can record source
locations in an `ObjectClass` for runtime diagnostics.

## class `lpcvm::Exception`

Declared in [vm_exception.hpp](../src/vm_exception.hpp).

`Exception<E>` adds LPC source context to a standard C++ exception while remaining
catchable as `E`. It uses the program and instruction address to identify the
source file, function, line, and column, and includes that context in `what()`.
When source information is unavailable, it preserves the original error message.

## class `lpcvm::Handle`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`Handle<T, RefType>` observes a reference-counted object without keeping it alive.
LPC object values use handles so that passing an object around does not extend
its lifetime. `expired()` checks whether the object still exists; `lock()` returns
an optional strong reference for access. The shared control block remains valid
until all strong and weak references are released.

## class `lpcvm::Sandbox`

Declared in [vm_sandbox.hpp](../src/vm_sandbox.hpp).

Manages a VM's named LPC objects. It loads and compiles source files into blueprint
objects, finds registered objects, and creates clones that share a compiled
`ObjectClass` but have separate state. The object table holds strong references;
callers receive weak handles. Explicit destruction removes the table's reference,
allowing the object to expire once any remaining strong references are released.

The constructor accepts a mudlib root for the intended resource boundary. The
current loader opens the supplied path directly; it does not yet enforce
filesystem confinement to that root.

## class `lpcvm::SharedPtr`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`SharedPtr<T>` owns a heap-allocated value through reference counting. Copies
share ownership, and releasing the last strong reference destroys the value.
It provides reference semantics for LPC arrays and mappings, and ownership of
registered LPC objects. The object and its prefix control block share one
allocation.

`SharedPtr` and `Handle` use non-atomic reference counts. All references to the
same object must stay on a single VM worker thread.

### function `lpcvm::create_shared`

`create_shared<T>(args...)` allocates an object with its control block, forwards
arguments to its constructor, and returns its initial `SharedPtr<T>`. Use this
factory to create values managed by LPCVM's reference-counted pointers.
