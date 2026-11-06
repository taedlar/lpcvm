# VM Fabric

The fabric supplies host runtime services to LPCVM execution: filesystem access,
shared strings, and object management. This document describes the fabric
implementations, their service contracts, and their use from C++.

See [LPCVM Ontology](lpcvm.md#fabric) for the fabric concept and object and value
lifetimes, and [Stack Virtual Machine Architecture](vm-architecture.md#fabric-boundary-and-language-policies)
for the execution boundary and possible language policy extensions.

`ManagedRuntime` provides the base RAII fabric. `lpcvm::Sandbox` extends it with
LPMud-style object loading, cloning, and filesystem services.

## RAII struct `ManagedRuntime`

Declared in [managed_runtime.hpp](../src/managed_runtime.hpp).

A lightweight RAII fabric that can be declared directly on the stack or as a unit
test fixture member. It needs no explicit initialization or shutdown. Scope exit,
including exception unwinding, releases its object and string table references.
It cannot be copied or moved because objects and execution contexts borrow its
address. External owning references may survive destruction, but a surviving
object's fabric pointer must no longer be used.

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
using its own name, `find_object()` retrieves a registered object by name, and `destruct_object()`
removes an object from the table. Callers receive weak handles; the object is destroyed
once all remaining strong references are released.

```cpp
ManagedRuntime fabric;
auto handle = LpcObject::create(&fabric, "std/player", nullptr); // creates and registers
auto found = fabric.find_object("std/player");  // returns the LpcObjectRef
fabric.emplace_object(found);  // registers using the object's own name
fabric.destruct_object(handle);  // removes from table
```

The object table is immutable from a caller's perspective through `objects()` const
member. `emplace_object(obj)` rejects null objects, absent or empty names, and objects
belonging to another fabric with `std::invalid_argument` before changing the table.
Replacing an existing name replaces both the object reference and the borrowed key.
The virtual method's overrides should preserve this contract. Derived classes access
the mutable table through `mutable_objects()` for custom implementations.
`LpcObject::create()` interns the name through the fabric and
stores an owning `LpcStringRef`; the object table borrows a view of those bytes.
`LpcObject::name()` returns an owning reference by value, allowing LPC variables
to share the string and retain it after object or fabric destruction. Unnamed
objects constructed directly have a null name reference; `create()` rejects empty
names. Unused interned names remain until sweeping or fabric destruction.

## class `lpcvm::Sandbox`

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

## Execution Integration

`ObjectClass::lookup_method()` accepts an optional fabric pointer as its fourth
argument, after the optional object state. The fabric may be a `ManagedRuntime`
or a derived `lpcvm::Sandbox`. Execution borrows this pointer, so the fabric must
remain alive for the duration of the call.

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
