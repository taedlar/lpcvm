# LPCVM
A sandboxed virtual machine inspired by LPMud's **blueprints and clones** architecture.

This is NOT a LPMud fork (it does not use LPMud source code). LPCVM is intended to be released with a permissive MIT license.
Blueprint-based OOP concepts and many other terms are borrowed from LPMud, and a deliberately minimal LPC-like (ANSI C like) language is developed for the VM.

## How To Build
From the top source directory:
~~~bash
cmake -S . -B build
cmake --build build
~~~

With `BUILD_TESTING=ON`, the unit tests and benchmarks are also built (see [Benchmarks](docs/development.md#benchmarks)).

## Overview
- **Sandboxed stack machine**: a compact bytecode VM whose mudlib sees only a portion of the filesystem. See [vm-architecture.md](docs/vm-architecture.md).
- **Objects**: each `*.lpc` file defines an object class. Loaded **blueprints** are cloned into objects with separate state, and blueprints can be hot-reloaded. Objects are destructed explicitly.
- **Values**: 16-byte `SValue` slots; primitives by value, compound types by reference-counted pointer, objects by weak handle. No raw pointers.
- **Thread-confined**: stacks, object state and reference counts need no synchronization; there is no global program counter or stack pointer.

## LPC Language
LPCVM implements a minimal [LPC grammar](src/lpc_parser.yy), aiming to stay
familiar to LPMud wizards. It provides efuns through native modules registered
by the host and made available to LPC code through `mount` declarations.

The minimal LPCVM runtime can start without any native modules. In that
configuration, LPC code has no access to the outside world. Hosts expose
capabilities by registering native modules; for example, `lpcshell` registers
`efun://console` to provide stdin and stdout access.

- Expressions: arithmetic, bitwise, comparison, unary, assignment (including compound forms), `++`/`--`, `?:`, and short-circuit `&&`/`||`.
- Statements: blocks, `if`/`else`, `while`, `do`/`while`, `for`, `break`, `continue`, and `return`.
- Tooling: `lpcfront` command-line frontend (dumps the AST), reentrant `flex`/`bison` preprocessor, lexer and parser, and a bytecode generator.

## Running lpcshell

`lpcshell` loads an LPC startup object from a sandbox root directory and calls
its `main()` method:

```text
lpcshell [--start FILE] [-V ...] ROOT
```

- `ROOT`: the sandbox root directory. Use `.` for the current directory.
- `--start FILE`: the startup source path within the sandbox root; defaults to
  `start.lpc`.
- `-V`, `--verbose`: increase logging verbosity; repeat for info (`-V`), debug
  (`-VV`), or trace (`-VVV`).
- `-h`, `--help`: show CLI help.
- `-v`, `--version`: show the version (lowercase `-v`).

[src/lpcshell/start.lpc](src/lpcshell/start.lpc) is a quick test of LPCVM. From
the repository root, after building with the commands above, run:

```bash
./build/src/lpcshell/lpcshell src/lpcshell
```

`lpcshell` compiles the startup file, then executes its `main()` method. The LPC
code prints a prompt, reads your name, and writes a greeting through the console
module. To run another startup file, specify its path relative to your sandbox:

```bash
./build/src/lpcshell/lpcshell --start examples/demo.lpc /path/to/mudlib
```

### Shell console EFUN

`lpcshell` registers `efun://console` before loading the startup object and
calling `main`. Mount it to access stdin and stdout:

```lpc
mount ("efun://console");

void main() {
    ::write("Name: ");
    mixed line = ::read_line();
    if (line)
        ::write("Hello, " + line + "\n");
}
```

- `mixed read_line()` reads through the next newline, returning the string
  without `\n`, or integer `0` at EOF. An empty line returns an empty string;
  a final line without a newline is returned before EOF.
- `int read_char()` reads one byte (0–255), including whitespace, or `-1` at EOF.
  It uses normal stdin buffering, so terminals may wait for Enter.
- `void write(string text)` writes the exact string and flushes stdout without
  adding a newline. Non-string arguments and I/O failures raise runtime errors.

## Documentation
- [docs/development.md](docs/development.md): build, unit-test, and benchmark workflows.
- [docs/lpcvm.md](docs/lpcvm.md): ontology of objects, methods, values, ownership, and runtime classes.
- [docs/vm-fabric.md](docs/vm-fabric.md): fabric implementations, runtime services, and C++ usage.
- [docs/vm-architecture.md](docs/vm-architecture.md): executor, bytecode instruction set, and sandbox architecture.
- [docs/lpc-language.md](docs/lpc-language.md): language grammar and compiler outline (work in progress).
- [docs/string-handling.md](docs/string-handling.md): string representations, measured assignment costs, and LPC programming guidance.
- [docs/string-runtime.md](docs/string-runtime.md): string ownership, temporary storage, accumulator bytecode, and compiler lowering.
