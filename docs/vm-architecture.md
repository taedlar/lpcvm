# LPCVM Stack Machine Architecture

LPCVM compiles LPC source into bytecode stored in an object class image. The image
also holds the constants and method table used to locate each method's bytecode
range. Blueprint and cloned objects can share this image while keeping separate
mutable object state.

## Executor

The host runs methods through a bounded executor view. An execution uses its
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
current instruction set is described below.

| Group | Opcodes | Behavior |
|-------|---------|----------|
| Stack | `F_PUSH_N`, `F_POP_N` | Allocate or discard stack slots (locals, discarded expression results). |
| Constants | `F_CONST_I4/I8/U4/U8/F4/F8`, `F_CONST_STR` | Push a constant-pool entry. |
| Variables | `F_LOAD_LOCAL`, `F_STORE_LOCAL`, `F_LOAD_GLOBAL`, `F_STORE_GLOBAL` | Stores keep the value on the stack, so an assignment is an expression. |
| Operators | `F_UNARY_OP`, `F_BINARY_OP` | The argument selects the operation (`oparg::UnaryOpArg`, `oparg::BinaryOpArg`). |
| Calls | `F_PUSH_FUNC`, `F_CALL`, `F_RETURN` | Method-table calls and frame teardown. |
| Jumps | `F_JUMP`, `F_JUMP_IF_FALSE`, `F_JUMP_IF_FALSE_OR_POP`, `F_JUMP_IF_TRUE_OR_POP` | Offsets are signed and relative to the next instruction. |

### Operators

`F_UNARY_OP` covers `!`, unary `-`, `~`, and increment/decrement (the latter
preserve the operand type, including floating point). Compound assignment
`a op= b` compiles to a load, the right-hand side, `F_BINARY_OP`, and a store.
Postfix `a++` leaves the original value as the result, while prefix `++a` leaves
the updated one.

`F_BINARY_OP` promotes mixed numeric operands to a common type: floating point
wins, same-signedness integers take the wider type, and mixed-sign integers
become `int64` (or `uint64` when the unsigned operand is 64-bit). Bitwise and
modulo operations require integer operands; shifts keep the left operand's
type. Division by zero raises a VM exception.

### Control flow

Jump instructions are emitted with a fixed-width placeholder argument and
patched once the target is known, so forward jumps never change instruction
sizes. `if`, loops, and the ternary `?:` use `F_JUMP_IF_FALSE`, which pops the
condition. Short-circuit `a && b` compiles to `a; F_JUMP_IF_FALSE_OR_POP end; b`
(`||` uses `F_JUMP_IF_TRUE_OR_POP`): if the jump is taken, `a` stays on the stack
as the result; otherwise it is popped and `b` is evaluated. The result is
therefore the last operand evaluated rather than a normalized 0 or 1. A value is
false if it is zero or void; every other value is true.

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
