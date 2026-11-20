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
3. `F_CALL_NATIVE`: get native function pointer from the resource provider, invoke function pointer

## `lpcvm::Execution`
1. Prepare bytecode of `ObjectClass`
2. Prepare data stack (arguments)
3. Prepare return stack
4. Call `execute()`, retrieve return value from the data stack
