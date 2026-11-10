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

Declared in [fabric.hpp](../src/fabric.hpp).

A lightweight RAII fabric usable on the stack for file and string services.
Object creation requires shared ownership, typically through
`std::make_shared<ManagedRuntime>()`. It needs no explicit initialization or
shutdown. Destruction releases its object and string table references. It cannot
be copied or moved. External owning references may survive destruction; a
surviving object's `retain_fabric()` returns empty after the runtime expires.

Provides default file access, shared-string services, and object table management
without requiring a sandbox. This is the base fabric for execution with or without
named object management. `make_shared_string()`, `shared_string_allocation_size()`,
`sweep_shared_strings()`, `emplace_object()`, `find_object()`, and `destruct_object()`
are virtual.

The free factory `lpcvm::make_shared_string(fabric, value)` forwards to the
virtual member. Custom storage implementations should override all three string
services consistently.

`file_search(name, search_paths)` is virtual and searches directories in order
for the first regular file, returning `std::optional<PathType>`.
The base runtime returns a canonical absolute host path. Absolute names are
checked directly; relative names require a search directory (use `"."` to search
the current directory). Missing files, dangling symlinks, and non-directory
search paths are skipped; no match returns `std::nullopt`. Other filesystem
errors are propagated as-is (e.g. `std::filesystem::filesystem_error`).
Empty names and embedded NUL bytes in names or searched directories throw `std::invalid_argument`.

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

`make_buffer(capacity)` acquires an empty mutable `LpcBuffer` (a `std::string`)
from a pool the runtime creates on first use. Copies of `LpcBufferRef` share
one buffer. When the last reference is released, the buffer is cleared and
recycled onto one of two free lists: buffers that fit in `std::string`'s inline
storage (`BufferPool::small_capacity()`, 15 bytes on libstdc++), and larger
buffers that keep their heap capacity. Requests up to the small capacity are
served only from the small list; larger requests take a large buffer and grow
it if needed. Default limits keep up to 1024 small and 64 large buffers, and
free buffers larger than 64 KiB rather than recycling them.
`buffer_pool_stats()` reports live and recycled counts, and `trim_buffer_pool()`
frees recycled buffers. Live buffers retain the pool and may outlive the
runtime. Buffers released after the runtime is destroyed are freed directly.

The object table holds strong references to all registered LPC objects and maps
their unique names to references. `emplace_object()` registers or updates an object
using its own name, `find_object()` retrieves a registered object by name, and `destruct_object()`
removes an object from the table. Callers receive weak handles; the object is destroyed
once all remaining strong references are released.

```cpp
auto fabric = std::make_shared<ManagedRuntime>();
auto handle = LpcObject::create(fabric, "std/player", nullptr); // creates and registers
auto found = fabric->find_object("std/player");  // returns the LpcObjectRef
fabric->emplace_object(found);  // registers using the object's own name
fabric->destruct_object(handle);  // removes from table
```

The object table is immutable from a caller's perspective through `objects()` const
member. `emplace_object(obj)` rejects null objects, absent or empty names, and objects
belonging to another or expired fabric with `std::invalid_argument` before changing the table.
Replacing an existing name replaces both the object reference and the borrowed key.
The virtual method's overrides should preserve this contract. Derived classes access
the mutable table through `mutable_objects()` for custom implementations.
`LpcObject::create()` interns the name through the fabric and
stores an owning `LpcStringRef`; the object table borrows a view of those bytes.
`LpcObject::name()` returns an owning reference by value, allowing LPC variables
to share the string and retain it after object or fabric destruction. Unnamed
objects constructed directly have a null name reference; `create()` rejects empty
names and null fabrics. Objects hold only a weak fabric reference, avoiding an
ownership cycle with the object table. Unused interned names remain until sweeping
or fabric destruction.

## class `lpcvm::Sandbox`

Declared in [vm_sandbox.hpp](../src/vm_sandbox.hpp).

A concrete fabric implementation that extends `ManagedRuntime` with object loading and
cloning services. It loads and compiles source files into blueprint objects, finds registered
objects through the inherited object table, and creates clones that share a compiled `ObjectClass`
but have separate state.

The constructor accepts a mudlib root. File opening, searching, and object
loading resolve paths within that root, including symlink targets. Absolute
paths refer to the mudlib root. `file_search()` returns canonical mudlib-relative
paths suitable for `file_open()`; an existing candidate resolving outside the
root throws `std::filesystem::filesystem_error` with `permission_denied`.
The host must prevent concurrent filesystem changes between validation and
access; this is not an OS-level filesystem jail.

`load_object()` compiles an LPC source file and registers it as a blueprint object.
`clone_object()` creates a new instance from an existing blueprint, sharing its
`ObjectClass` but with separate state. Both operations register the object in the
inherited object table from `ManagedRuntime`.
These operations require a shared-owned sandbox, created with
`std::make_shared<lpcvm::Sandbox>(root)`. Calling object loading or cloning without
shared ownership throws `std::bad_weak_ptr` when creating an object. File and
string services remain usable with a stack-owned sandbox.

Shared-string allocation, accounting, and sweeping are inherited from
`ManagedRuntime` and remain available through the sandbox. The object table
inherited from `ManagedRuntime` holds strong references to all named objects
in this sandbox's filesystem domain.

## Execution Integration

A `lpcvm::Execution` accepts and retains a `std::shared_ptr<ManagedRuntime>`.
Copies of the execution share ownership; destruction or rebinding releases its
reference. `binding(fabric, bytecode, size, program)` replaces both the fabric
and bytecode binding.

`LpcObject` holds a weak fabric reference to avoid ownership cycles with the
runtime's object table. `retain_fabric()` locks that reference; object-backed
executions acquire it automatically, even for objects without a program or state.
Stack-owned runtimes remain supported for file and string services, but cannot be
passed to the object factory.

Each object uniquely owns its `ObjectState`; `state()` returns a borrowed pointer
that remains valid while the object lives. State has no fabric reference.
`as_with(&state)` binds the state without changing the execution's fabric.

String literals emit `F_CONST_STR`, which loads an owning string from the
constant pool without requiring an execution fabric. Storing a temporary string
in object state requires a live execution fabric to allocate an owning string.
Owning strings remain valid after the execution, program, or fabric is destroyed.

```cpp
auto fabric = std::make_shared<ManagedRuntime>(); // A shared Sandbox also works.
ObjectState state(program->num_variables_inheriting());
lpcvm::Execution entry(fabric);
if (program->lookup_method("set_message", &entry))
    entry.as_with(&state).execute(control, stack);
```
