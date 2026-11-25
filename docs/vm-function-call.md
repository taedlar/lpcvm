# Function Calling

In LPCVM, the following function calling scenarios are provided:

Scenario | Caller | Callee | Description
--- | --- | --- | ---
`F_CALL` | Bytecode | Bytecode | LPC code with static binding (local functions and inherited functions)
`F_CALL_OTHER` | Bytecode | Bytecode | LPC code with runtime binding
`F_CALL_NATIVE` | Bytecode | Native Code | Efuns / Plug-ins
`lpcvm::Execution` | Native Code | Bytecode | lpcshell, MUD driver

## `F_CALL`
1. Push function index
2. Push arguments
3. `F_CALL`: push control frame, jump to begin of function bytecode

## `F_CALL_OTHER`
1. Push object reference
2. Push function name as shared string
3. Push arguments
4. `F_CALL_OTHER`: resolve remote function binding by name, push control frame, jump to begin of function bytecode

## `F_CALL_NATIVE`
1. Push resource descriptor and function name
2. Push arguments
3. `F_CALL_NATIVE`: acquire the named resource from the execution fabric, then
   dispatch through `NativeModule::call_method`, which resolves and invokes the
   registered native function pointer.

### Function-local native mounts

```lpc
int example(string module_name) {
    mount (module_name => Module);
    return Module::add(2, 3);
}
```

The host registers a named `NativeModule` with `ManagedRuntime::emplace_object`.
Its methods must be registered with `add_method` and `finalize_methods` before
calling them. Module names use the fabric's object-table namespace verbatim.

`mount` evaluates its path and binds a nonempty string to a local descriptor;
it does not look up or retain the registered module. `F_BIND_MOUNT slot` consumes
the path and stores its immutable, fabric-interned name at `mnt0_ + slot`.
The mount body runs after binding; resource availability does not select the
`else` branch. Invalid binding values raise an execution error.

For `Module::method(args)`, `F_LOAD_MOUNT slot` pushes the bound name, followed by
the method name and arguments. The `F_CALL_NATIVE` operand counts all these
stack values (two names plus actual arguments). It acquires the current object
registered under that name and requires a `NativeModule`. Each call looks up the
registration again, so replacing or removing a module affects subsequent calls.
The acquired strong reference protects a module through callbacks, including a
callback that replaces its own registration. The method name is interned in the
execution fabric to match the native method table's pointer identities.

The native module normalizes arguments using its method signature. Keyword
arguments are packed into a trailing mapping, following `NativeModule`'s existing
vararg convention. Native callbacks consume their arguments and return the number
of results. LPC native calls accept zero or one result; zero becomes a void
expression value. The executor removes both name operands and checks the returned
stack size and return-stack depth. Missing modules or methods, unbound descriptors,
non-native resources, and invalid callback stack results raise execution errors.
Owning references created for acquisition are released on return or exception.

This first implementation lowers explicit function-local aliases. Class mounts,
anonymous `::` dispatch, and LPC resource calls are not implemented by this opcode.

## `lpcvm::Execution`
1. Prepare bytecode of `ObjectClass`
2. Prepare data stack (arguments)
3. Prepare return stack
4. Call `execute()`, retrieve return value from the data stack
