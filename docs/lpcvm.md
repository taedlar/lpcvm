# LPCVM Ontology

This document describes LPCVM entities, their relationships, and their lifetimes:
objects, methods, values, and the runtime types that represent them.

See [vm-architecture.md](vm-architecture.md) for the stack machine, bytecode, and
sandbox architecture, [lpc-language.md](lpc-language.md) for the language and
compiler outline, and [development.md](development.md) for build, unit-test, and
benchmark workflows.

## Objects

- Objects are defined with LPC language. Each LPC source file (`*.lpc`) defines an **object class** with method functions and variables.
- The LPC compiler compiles source files to object class images (function bytecodes, constant pool, and virtual table of method functions).
- The object loader **loads** an object by invoking the LPC compiler to compile LPC source files in-memory.
  - The object class image is reference-counted, with the first reference held by the loaded object.
  - The loaded object is registered with a unique name (e.g. `"std/player"`) and serves as a **blueprint object**.
  - More objects can be **cloned** from blueprint objects, adding more references to their object class image.
- Hot-reloading replaces the registered blueprint with a newer version. Existing clones continue to use the old version.
- Each object has its own **object state** (variables, I/O buffers, and other attributes).
  - The LPC `object` data type is a handle to an individual object and allows access to its state.
  - The LPC `call_other` efun calls methods defined in the object class image referenced by the object.
  - The virtual machine can also invoke methods on I/O events, timers, or policy contracts.
  - An LPC object must be **destructed** explicitly once loaded, to emulate physical objects (e.g. monsters) in a virtual world.

## Method Functions

Methods share their bytecode and constants through an `ObjectClass`; object
variables belong to the target object's `ObjectState`. Each method binding records
its signature and bytecode range. `ObjectClass::lookup_method()` resolves a name
to an `Execution` with the class context. Its optional third and fourth arguments
attach an `ObjectState*` and a fabric pointer (either a `ManagedRuntime*` or `Sandbox*`), respectively.

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
- [lpcvm::Sandbox](#class-lpcvmsandbox)
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
String literals emit `F_CONST_STR`, which interns the constant-pool bytes through
`make_shared_string()` in the supplied fabric and pushes an owning reference.
Executing this opcode without a fabric throws. Returned strings and strings
stored in object state remain valid after the execution, program, or fabric is
destroyed, as long as their owning references remain alive.

```cpp
ManagedRuntime fabric; // A Sandbox can also be supplied.
lpcvm::Execution entry;
if (program->lookup_method("set_message", &entry, &state, &fabric))
    entry.execute(control, stack);
```

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

### class `ManagedRuntime`

Declared in [managed_runtime.hpp](../src/managed_runtime.hpp).

Provides default file access, shared-string services, and object table management
without requiring a sandbox. This is the base fabric for execution with or without
named object management. `make_shared_string()`, `shared_string_allocation_size()`,
`sweep_shared_strings()`, `emplace_object()`, `find_object()`, and `destruct_object()`
are virtual.

The free factory `lpcvm::make_shared_string(fabric, value)` forwards to the
virtual member. Custom storage implementations should override all three string
services consistently.

The default fabric interns immutable shared strings. Equal byte sequences created
through `lpcvm::make_shared_string(fabric, value)` share one `LpcString` within
that fabric. The factory copies the input, including embedded NUL bytes.
The table holds a strong reference until sweeping or fabric destruction;
external owning references remain valid after the fabric is destroyed.

```cpp
ManagedRuntime fabric;
auto text = lpcvm::make_shared_string(fabric, "hello");
auto bytes = fabric.shared_string_allocation_size(); // 22 on the 64-bit build
text = {};
auto removed_bytes = fabric.sweep_shared_strings(); // 22
```

`shared_string_allocation_size()` sums the allocation sizes of distinct strings
currently in the table: reference-count block, string metadata, characters, and
one trailing NUL byte (17 + length on the 64-bit build). It excludes allocator
and hash-table overhead. `sweep_shared_strings()` removes entries whose only
strong reference belongs to the table and returns their total allocation size.
Both operations take linear time in the table size and run on the owning thread.
Sweeping retains hash buckets; outstanding weak handles can delay the physical
release of removed strings' allocations.

The object table holds strong references to all registered LPC objects and maps
their unique names to references. `emplace_object()` registers or updates an object
by name, `find_object()` retrieves a registered object by name, and `destruct_object()`
removes an object from the table. Callers receive weak handles; the object is destroyed
once all remaining strong references are released.

```cpp
ManagedRuntime fabric;
auto obj = create_shared<LpcObject>(...);
auto handle = fabric.emplace_object("std/player", obj);
auto found = fabric.find_object("std/player");  // returns the LpcObjectRef
fabric.destruct_object(handle);  // removes from table
```

The object table is immutable from a caller's perspective through `objects()` const
member. Derived classes access the mutable table through `objects()` for custom
implementations. Object names are stored as references to interned strings, so the
lifetime of the name reference is tied to the object itself.

### class `lpcvm::Sandbox`

Declared in [vm_sandbox.hpp](../src/vm_sandbox.hpp).

A concrete fabric implementation that extends `ManagedRuntime` with object loading and
cloning services. It loads and compiles source files into blueprint objects, finds registered
objects through the inherited object table, and creates clones that share a compiled `ObjectClass`
but have separate state.

The constructor accepts a mudlib root for the intended resource boundary. The
current loader opens the supplied path directly; it does not yet enforce
filesystem confinement to that root. File operations (`file_open()`) are confined
to paths within the mudlib root.

`load_object()` compiles an LPC source file and registers it as a blueprint object.
`clone_object()` creates a new instance from an existing blueprint, sharing its
`ObjectClass` but with separate state. Both operations register the object in the
inherited object table from `ManagedRuntime`.

Shared-string allocation, accounting, and sweeping are inherited from
`ManagedRuntime` and remain available through the sandbox. The object table
inherited from `ManagedRuntime` holds strong references to all named objects
in this sandbox's filesystem domain.

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

## Object Fabric Design

The object fabric is a service layer that manages named LPC object lifecycles,
independent of how objects are created or persisted. This design separates
concerns and enables flexible object management strategies.

### Architecture

- **ManagedRuntime**: The base fabric providing object table management. The table
  is a name-to-reference map that holds strong references to all registered objects.
  Methods `emplace_object()`, `find_object()`, and `destruct_object()` manage
  registration and destruction. `objects()` provides an immutable view of the table.

- **Sandbox**: A specialized fabric that adds file-based object loading and cloning.
  It inherits the object table from `ManagedRuntime` and adds `load_object()` and
  `clone_object()` for filesystem-backed objects. Multiple Sandboxes could exist
  with different filesystem roots, each maintaining its own object table.

### Separation of Concerns

The object fabric distinguishes between three functions:

1. **Table management** (in `ManagedRuntime`): Registering, finding, and destroying
   named objects. The table holds strong references; objects are destroyed when
   explicitly removed from the table and all remaining references expire.

2. **Object loading and compilation** (in `Sandbox`): Compiling LPC source files
   to object class images and instantiating blueprint objects. These operations
   integrate with the table inherited from `ManagedRuntime`.

3. **Object naming and location** (external to both): LPC source file paths are
   normalized to object names (e.g., `"std/player"`), and cloned objects are
   named with `#` suffixes. This knowledge is provided by the Sandbox's caller,
   not embedded in the runtime.

### Extensibility

Custom fabrics can inherit from `ManagedRuntime` and override `emplace_object()`,
`find_object()`, and `destruct_object()` to implement different object lifecycle
strategies (e.g., database-backed objects, transient object pools, or permission-based
access control). The base table management remains available for numeric-only
execution or minimal object scenarios.

Objects loaded or created through custom fabrics use weak reference handles, the
same as Sandbox objects. The fabric supplies the strong reference lifecycle;
the executor and LPC code see only the weak-reference interface.
