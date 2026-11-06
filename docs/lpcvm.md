# LPCVM Ontology

This document describes LPCVM entities, their relationships, and their lifetimes.

## Fabric

LPCVM is not a hypervisor-level virtual machine. It is a **fabric** of bytecode,
data entities, and managed runtime resources. The fabric defines the substance
of environment in which LPC code executes, and the boundary between each other
in the virtual machine and its host.

The boundary works in three directions:

- LPC object-to-object relationships and method calls.
- LPC code requests host resources or calls native C++ functions (efuns).
- The C++ host invokes LPC code to handle events or provide application behavior.

The fabric supplies filesystem access, shared strings, and an object table. Its
resource and policy boundaries depend on the fabric implementation. Objects and
execution contexts are associated with a fabric; owning references to values may
outlive it according to the lifetime rules below.

See [VM Fabric](vm-fabric.md) for implementations, service contracts, and usage.

## Objects

- Objects are defined with LPC language. Each LPC source file (`*.lpc`) defines an **object class** with method functions and variables.
- The LPC compiler compiles source files to object class images (function bytecodes, constant pool, and virtual table of method functions).
- LPCVM can **load** an object by invoking the LPC compiler to compile LPC source files in-memory.
  - The compiled object class image has reference-counted lifecycle, with the first reference held by the loaded object.
  - The loaded object is registered with a unique name (e.g. `"std/player"`) in the **object table** owned by the fabric.
  - The object holding the initial reference to an object class is a **blueprint object**.
- LPCVM can **find** a loaded object by its registered name.
- LPCVM can **clone** objects from a blueprint object, sharing the same object class image by reference.
- The object table own the primary reference to objects, while C++ code may own additional temporary reference to objects.
  The LPC `object` data type does not imply ownership of object, it is a handle (weak reference).
- LPCVM can **destruct** a loaded object by erasing the primary reference in object table.
  The destruction take place when all references held by C++ code are released.
- Hot-reloading replaces the registered blueprint object in object table with a newer version. Existing clones continue to use the old version.
- Each object has its own **object state** (variables, I/O buffers, and other attributes).
  - The LPC `object` data type is a handle to an individual object and allows access to its state.
  - The LPC `call_other` efun calls methods defined in the object class image referenced by the object.
  - The virtual machine can also invoke methods on I/O events, timers, or policy contracts.
  - An LPC object must be **destructed** explicitly once loaded, to emulate physical objects (e.g. monsters) in a virtual world.

## Method Functions

Methods share their bytecode and constants through an `ObjectClass`; object
variables belong to the target object's `ObjectState`. Each method binding records
its signature and bytecode range. `ObjectClass::lookup_method()` resolves a name
to an `Execution` with the class context.

Calls within a class use method-table indices. The caller pushes the method index
and arguments, and the executor saves the caller's frame on the return stack.
Arguments, locals, and temporary values occupy the data stack. Returning removes
the callee's frame while leaving its return values for the caller.

## Variables

A variable holds an `SValue`: a value that fits in one stack element, such as a primitive value, a reference to a compound value, or an object handle.

- Numeric types (`int`, `float`, ...) are stored in the `SValue` and passed by value.
- Strings use owning `LpcStringRef` values pointing to immutable shared bytes.
- Compound types (`function`, `mapping`, arrays) live in heap memory with reference-counted lifecycle and are passed by reference.
- Objects live in heap memory and are passed by handle (weak reference).
- There is no raw pointer semantics in LPC.
- Object variables are destroyed with the object, stack variables (arguments and locals) when popped, and compound values when their reference count reaches zero.

## LPC Memory Model

LPCVM uses standard C++ containers and smart pointers for internal data structures.
LPC values follow the ownership and lifetime rules below.

### SValue

LPCVM represents runtime values using `SValue`, a tagged `std::variant`. Each
slot can hold any supported runtime value: a numeric value, an owning reference,
an object handle, or an empty value. This storage representation does not itself
define the language's rules for declared types.

`SValue` occupies 16 bytes in the current 64-bit implementation. Numeric values
are stored directly; copying a reference or handle preserves its ownership
semantics rather than copying the referenced object.

### Compound Data Types

Arrays, mappings, and closures are intended to use **strong reference** semantics:
copies share a value, and releasing the last strong reference destroys it.
Array and mapping reference types currently use `lpcvm::SharedPtr`, but their
container implementations are placeholders. Closures are not yet represented in
`SValue`.

Strings use `LpcStringRef`, an owning `lpcvm::SharedPtr` to immutable shared bytes.
The runtime's intern table also retains a strong reference until the entry is
swept or the runtime is destroyed. Other owning references keep the string alive
independently of the runtime.

`SharedPtr` stores its reference counts in a control block co-allocated before
the value. This keeps each reference small enough to fit in the 16-byte `SValue`.
Reference counts are non-atomic: all strong and weak references to the same
allocation must remain on one VM worker thread.

### Objects

LPC object values use **weak reference** semantics, represented by
`lpcvm::Handle`. Copying an object handle does not extend the object's lifetime.
The runtime's object table holds a strong reference to each registered object
through `emplace_object()`. Explicit destruction via `destruct_object()` removes
that reference; the object is destroyed once all remaining strong references are
released.

C++ code accesses an object through `Handle::lock()`, which returns an optional
`lpcvm::SharedPtr`. Locking an expired handle fails, so callers must check the
result before dereferencing it. Weak handles keep the control block valid after
the object's lifetime ends, until the last weak reference is released.

This model allows LPC objects to represent external resources, such as a player
connection, without making their lifetimes depend on every LPC variable that
refers to them. The runtime's object table can be shared across multiple fabric
implementations or scopes, enabling flexible object lifecycle management.

### Exceptions

LPCVM supports RAII and native C++ exceptions. Bytecode execution reports runtime
errors through standard exceptions such as `std::runtime_error`. C++ stack
unwinding destroys local RAII objects and releases the resources they own.

On an exception, bytecode execution leaves caller-owned object state, data stacks,
and return stacks available for diagnostic trace dumps to assist LPC debugging.
It does not roll back state changes or automatically reset the VM stacks; their
contents reflect execution up to the failure. Values retained in those stacks
remain alive until the caller removes them or destroys the stacks.

## Virtual Machine

The `lpcvm` namespace provides bytecode generation, runtime diagnostics, object
management, and reference-counted ownership through the following types:

- [lpcvm::Bytecode](#class-lpcvmbytecode)
- [lpcvm::DataStack](#class-lpcvmdatastack)
- [lpcvm::Exception](#class-lpcvmexception)
- [lpcvm::Execution](#struct-lpcvmexecution)
- [lpcvm::Handle](#class-lpcvmhandle)
- [lpcvm::ReturnStack](#class-lpcvmreturnstack)
- [lpcvm::SharedPtr](#class-lpcvmsharedptr)

### class `lpcvm::Bytecode`

Declared in [code_generator.hpp](../src/code_generator.hpp).

Builds VM bytecode in a caller-owned byte vector. The LPC compiler uses it to
translate AST expressions, statements, and function bodies into stack-machine
instructions. It handles extended instruction arguments and can record source
locations in an `ObjectClass` for runtime diagnostics.

### class `lpcvm::DataStack`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Stores arguments, locals, and expression operands as a contiguous vector of
`SValue`s. The executor addresses frame slots by index so stack growth does not
invalidate the frame pointer. Push and pop operations manage value lifetimes;
invalid indices and stack underflow raise exceptions. The caller owns the stack
and can reuse its capacity across executions.

### class `lpcvm::Exception`

Declared in [vm_exception.hpp](../src/vm_exception.hpp).

`Exception<E>` adds LPC source context to a standard C++ exception while remaining
catchable as `E`. It uses the program and instruction address to identify the
source file, function, line, and column, and includes that context in `what()`.
When source information is unavailable, it preserves the original error message.

### RAII struct `lpcvm::Execution`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Holds a bytecode range and non-owning pointers to its class, optional object
state, and optional fabric, which must remain alive during execution.
`execute()` runs synchronously
with caller-supplied data and return stacks. Arguments are already on the data
stack; normal return removes them and the locals, leaves the results, and reports
the number of return values. Object-variable access requires an object state.
See [VM Fabric](vm-fabric.md#execution-integration) for fabric services used during execution.

The program counter and frame pointer are local to the call; this type does not
store a suspended continuation.

### class `lpcvm::Handle`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`Handle<T, RefType>` observes a reference-counted object without keeping it alive.
LPC object values use handles so that passing an object around does not extend
its lifetime. `expired()` checks whether the object still exists; `lock()` returns
an optional strong reference for access. The shared control block remains valid
until all strong and weak references are released.

### class `lpcvm::ReturnStack`

Declared in [vm_executor.hpp](../src/vm_executor.hpp).

Stores caller frames for nested method calls. Each frame saves the return address,
data-stack frame index, and caller's bytecode bounds. Returning restores this
context and pops the frame. `execute()` records the initial return-stack depth so
normal completion preserves any frames supplied by its caller. Return values
remain on the data stack.

### class `lpcvm::SharedPtr`

Declared in [lpc_types.hpp](../src/lpc_types.hpp).

`SharedPtr<T>` owns a heap-allocated value through reference counting. Copies
share ownership, and releasing the last strong reference destroys the value.
It provides reference semantics for LPC arrays and mappings, and ownership of
registered LPC objects. The object and its prefix control block share one
allocation.

`SharedPtr` and `Handle` use non-atomic reference counts. All references to the
same object must stay on a single VM worker thread.

### factory function `lpcvm::create_shared`

`create_shared<T>(args...)` allocates an object with its control block, forwards
arguments to its constructor, and returns its initial `SharedPtr<T>`. Use this
factory to create values managed by LPCVM's reference-counted pointers.
