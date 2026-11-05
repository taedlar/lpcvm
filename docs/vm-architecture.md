# Stack Virtual Machine Architecture

The stack VM executes bytecode as language-neutral target code. A compiler maps
its source language into the VM's instructions, value representation, and calling
convention. The executor operates on stack slots, constant-pool entries, state
slots, and function bindings; source grammar and high-level language policies
belong to the compiler and host runtime.

LPC is the current frontend. Language neutrality describes the execution boundary,
not a claim that other language frontends already exist. A frontend must honor
the VM's concrete type, operator, and control-flow semantics, or lower its own
semantics into suitable instruction sequences and runtime services.

See [lpc-language.md](lpc-language.md) for the evolving LPC language and compiler,
[lpcvm.md](lpcvm.md) for runtime classes and object lifetimes, and
[development.md](development.md) for build, test, and benchmark workflows.

## Execution Inputs

An `Execution` view associates a bytecode range with an optional program image,
mutable state, and fabric. The host supplies a `DataStack`, a `ReturnStack`, and
the number of arguments already on the data stack when invoking `execute()`.

The current program-image container is named `ObjectClass`. It holds bytecode,
constants, and method bindings that locate function bytecode ranges. `ObjectState`
holds mutable value slots separately, so multiple instances can share program
code and constants. Blueprint registration, cloning, and hot-reloading are
higher-level uses of this separation, described in [lpcvm.md](lpcvm.md).

Each instruction determines which inputs it needs: constant loads require a
constant pool, global-slot access requires mutable state, and string creation
requires a fabric. Numeric-only execution can omit the fabric.

## Data Stack and Values

`DataStack` holds `SValue` slots for arguments, locals, intermediate operands, and
return values. `SValue` is a compact tagged variant: primitive values are stored
directly, compound values use reference-counted storage, and object values use
handles. The same representation is used for mutable state slots.

The frame pointer identifies the first argument of the active function. Local
loads and stores address `fp + index`; `F_PUSH_N` allocates additional slots for
locals or temporary storage. Global loads and stores address the attached state
using a state offset and instruction argument. These are instance state slots,
not process-wide global variables.

Most operations consume operands from the top of the stack and leave a result.
For example, two constant loads followed by `F_BINARY_OP ADD` change the stack
from `[..., left, right]` to `[..., sum]`. Stores copy the top value into the
selected slot while leaving that value on the stack; `F_POP_N` discards values
that are no longer needed.

## Call Frames and Returns

The executor maintains a program counter, frame pointer, active bytecode bounds,
and state offset. `ReturnStack` stores the caller's continuation separately from
operand values: the return program counter, frame pointer, state offset, and
bytecode bounds.

`F_PUSH_FUNC` places a method-table index on the data stack. The arguments follow
that index, and `F_CALL` specifies their total slot count, including the index.
It resolves the callee's binding, saves the caller's execution state, establishes the
callee's frame, and switches to its bytecode range. Calls are dispatched by the
VM loop rather than by recursively invoking the C++ executor.

`F_RETURN` specifies the number of values to preserve at the top of the stack.
It removes the completed frame's arguments, locals, and temporaries, then restores
the caller's continuation. Returning from the entry frame transfers control to
the host and reports the number of return values remaining on the data stack.
Execution state belongs to the invocation and its stacks; there is no process-wide
program counter or stack pointer.

## Bytecode Encoding and Dispatch

Each instruction consists of an 8-bit opcode and an 8-bit argument.
`F_EXTENDED_ARG` prefixes supply higher argument bytes, allowing the following
instruction to assemble a wider argument. The executor uses a 32-bit argument
accumulator and resets it after dispatching an ordinary instruction.

The dispatch loop uses computed goto with GCC and Clang, with a switch-based
fallback for other compilers. Both dispatch paths implement the instruction set
in [vm_opcodes.hpp](../src/vm_opcodes.hpp); execution handlers live in
[vm_executor.cpp](../src/vm_executor.cpp).

| Group | Opcodes | Stack or execution effect |
|-------|---------|---------------------------|
| Argument extension | `F_EXTENDED_ARG` | Accumulate higher bytes of the next instruction argument. |
| Stack | `F_PUSH_N`, `F_POP_N` | Allocate or discard value slots. |
| Constants | `F_CONST_I4/I8/U4/U8/F4/F8`, `F_CONST_STR` | Push a typed constant-pool entry. |
| Storage | `F_LOAD_LOCAL`, `F_STORE_LOCAL`, `F_LOAD_GLOBAL`, `F_STORE_GLOBAL` | Load a slot or copy the top value into it; stores preserve the top value. |
| Operators | `F_UNARY_OP`, `F_BINARY_OP` | Replace one or two operands with an operation result. |
| Calls | `F_PUSH_FUNC`, `F_CALL`, `F_RETURN` | Select a function, enter its frame, or return values to the caller. |
| Branches | `F_JUMP`, `F_JUMP_IF_FALSE`, `F_JUMP_IF_FALSE_OR_POP`, `F_JUMP_IF_TRUE_OR_POP` | Change the program counter, optionally testing or removing the top value. |

### Operator Semantics

The argument to `F_UNARY_OP` or `F_BINARY_OP` selects the operation. Unary
operations include logical negation, numeric negation, bitwise complement, and
increment/decrement. Increment and decrement preserve the operand's numeric type,
including floating point; operand support is defined by the individual handler.

Binary numeric operations promote operands to a common type: floating point
takes precedence, same-signedness integers take the wider type, and mixed-sign
integers become `int64`, or `uint64` when the unsigned operand is 64-bit. Bitwise
and modulo operations require integer operands; shifts retain the left operand's
type. Division by zero raises a VM exception. These are target-code semantics
that a frontend must account for when compiling expressions.

### Branch Semantics

Branch offsets are signed byte displacements relative to the next instruction.
Targets are checked against the active function's bytecode range.

- `F_JUMP` branches unconditionally.
- `F_JUMP_IF_FALSE` consumes its condition and branches when it is false.
- `F_JUMP_IF_FALSE_OR_POP` and `F_JUMP_IF_TRUE_OR_POP` preserve the top value when
  taking the branch, and pop it when continuing to the next instruction.

Numeric zero and void are false; other values are true. These primitives support
conditional execution, loops, and value-preserving short-circuit evaluation
without encoding source-language statement forms in the instruction set.

## Fabric Boundary and Language Policies

The fabric supplies host runtime services beyond stack and bytecode mechanics.
`Execution` accepts a `ManagedRuntime` pointer. `ManagedRuntime` provides
shared-string allocation, filesystem access, and object table management.
`F_CONST_STR`, for example, obtains an owning string through the fabric before
pushing it onto the stack.

The object table is a core fabric service that maintains strong references to all
registered LPC objects and maps their unique names to references. This enables
execution without filesystem or compilation services (numeric-only execution with
objects), and supports fabric implementations that manage object lifetimes
differently than a traditional LPC mudlib. `Sandbox` extends `ManagedRuntime` with
a filesystem root and object loading/cloning services specific to file-based object
management.

This boundary could be extended with host calls, I/O and network events, and
high-level language policy hooks. An LPC runtime could use such hooks to consult
a **master object** for policies governing resource access, object loading, or
privileged operations. The host service would receive the request and caller
context, consult the policy, and perform or reject the operation. The executor
would continue to implement the same low-level stack and call mechanics.

Master-object policy hooks are a possible extension, not an implemented contract.
Their design would need to define caller identity, policy invocation, and failure
propagation at the fabric boundary. Other language runtimes could supply their
own policies through that boundary without making LPC-specific policy rules part
of the bytecode instruction set.

## Execution Ownership and Future Scheduling

Stacks, mutable state, and reference counts are thread-confined. An invocation
must retain access to its program image, state, and fabric for the duration of
execution. Runtime checks detect conditions such as stack underflow, invalid
opcodes, and out-of-range branch targets; they do not constitute a complete
verifier for arbitrary bytecode.

Virtual threading remains a long-term goal and is not supported by the current
executor. Suspension would need to preserve the program counter, frame state,
data stack, and return stack. Scheduling must prevent concurrent access to the
same mutable state or stacks. Continuations holding thread-confined references
would need to stay on their owning worker or use an explicit safe handoff mechanism.
