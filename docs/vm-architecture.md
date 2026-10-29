# LPCVM Stack Machine Architecture

LPCVM compiles LPC source into bytecode stored in an object class image. The image
also holds the constants and method table used to locate each method's bytecode
range. Blueprint and cloned objects can share this image while keeping separate
mutable object state.

## Runtime

The host runs methods through a bounded bytecode view. An execution uses its
object class and, when needed, the target object's state, along with a data stack
and a return stack supplied by the caller. The program counter and frame pointer
are local to the active execution; call frames save the caller's position and
restore it when a method returns.

Arguments and temporary values occupy the evaluation stack as `SValue`s. A frame
starts at its arguments, with locals allocated in the same stack area. Bytecode
loads and stores locals or object variables, applies operations to stack values,
and calls methods through their method-table index. Returning removes the frame's
temporary values while preserving its declared return values.

## Sandbox

The sandbox is the host boundary for filesystem visibility, I/O and network
events, and C++ entry points. Runtime resources are represented as LPC objects;
the host decides which objects and operations are exposed.

## Objects and State

An LPC object combines a reference to an object class with its own object state.
Registered blueprints provide the class image used to create clones. Replacing
a blueprint supports hot-reloading: existing objects can continue using their
previous class image. Object variables live in the per-object state; method
bytecode and constants are shared by objects using the same image.

## SValue

`SValue` is a compact `std::variant` slot for LPC values. It is used both for
evaluation-stack operands and object attributes. Primitive values are stored
directly; compound values use reference-counted storage, and object values are
handles. Stack and object state are thread-confined, so access requires no
synchronization.

## Bytecode

Bytecode is shared through the object class image. Each instruction is an
8-bit opcode followed by an 8-bit argument; `F_EXTENDED_ARG` prefixes extend
the following instruction's argument when it needs more than one byte. The
current instruction set covers stack management, constants, local and object
variable access, binary operations, method calls, and returns.

The executor consumes a method's bytecode range with its object state, data
stack, and return stack. It has no process-wide program counter or stack pointer;
execution state belongs to the active invocation and its stacks.

## Virtual Threading

Virtual threading is an advanced long-term goal for LPCVM; it is not supported
by the current executor. A virtual thread would need to preserve its execution
continuation, including the program counter, frame state, data stack, and return
stack, across suspension and resumption. Scheduling across workers must prevent
concurrent access to the same mutable object state or execution stacks. Since
LPC reference counts are thread-confined, virtual threads must also remain on
their owning worker while holding LPC references, or use an explicit safe
cross-worker handoff mechanism.
