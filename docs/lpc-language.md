# LPC Language and Compiler

> [!NOTE]
> The LPC language grammar and compiler are under active development. This
> document is a high-level outline, not a complete or stable language specification.

## Design Direction

LPCVM starts from a minimal LPC grammar, aiming for a language familiar to LPMud
wizards with selected enhancements and simplifications. Extended functionality
is provided through efuns. The grammar and tests track the evolving implementation.

## Language Outline

### Source Structure and Declarations

Each LPC source file defines an object class with variables and method functions.
The [parser grammar](../src/lpc_parser.yy) defines the accepted syntax, while the
[lexer](../src/lpc_lexer.l) defines tokens. See [lpcvm.md](lpcvm.md) for the object
and value model.

### Expressions and Types

The current expression set covers arithmetic, bitwise and comparison operators,
unary operators, assignment and compound assignment, increment and decrement,
conditional expressions, and short-circuit logical operators.

- `&&` and `||` return the last evaluated operand rather than a normalized 0 or 1.
- Mixed numeric operands are promoted to a common type: floating point takes
  precedence, and mixed-sign integers become `int64`, or `uint64` when the
  unsigned operand is 64-bit.

### Statements and Control Flow

The current statement set covers blocks with local variables, `if`/`else`,
`while`, `do`/`while`, `for`, `break`, `continue`, and `return`.

## Compiler Outline

### Preprocessing

The reentrant preprocessor handles macro expansion before parsing. Its
implementation lives in [preprocessor.l](../src/preprocessor.l); macro-expansion
benchmarks are described in [development.md](development.md#macro-expansion).

`#include "file.h"` and `#include <file.h>` preprocess a header in place, sharing
macro definitions with the including source. Macro operands such as
`#define HEADER "file.h"` followed by `#include HEADER` are supported.
Configure the ordered search directories before `preprocess()` or `parse()`:

```cpp
LpcCompiler compiler;
compiler.use_include_directories({"include", "lib/include"});
compiler.set_filename("src/main.lpc");
compiler.parse(source);
```

Each call to `use_include_directories()` replaces the list. Quoted includes first
search the directory of the including file (using `set_filename()` for the main
source), then the configured directories. Angle-bracket includes search only the
configured directories. There are no implicit directories for an unnamed source.
Absolute include names are checked directly through the fabric.

All include searches and opens use `use_fabric()`'s runtime; without one, the
compiler creates a `ManagedRuntime`. Relative host directories use the process
working directory. With a `Sandbox`, absolute paths refer to the mudlib root and
traversal and symlink checks apply. The sandbox object loader supplies its own
fabric and the object's filename so local quoted includes work safely.

Missing or malformed includes, active include cycles, and nesting beyond 64
headers fail preprocessing/parsing and populate `messages()`. Preprocessor
errors in headers include their filename and original line/column in the message.
Conditional directives and `#pragma once` are not implemented.

`#line N` sets the logical line number of the following line; an optional quoted
filename (`#line 42 "generated.lpc"`) also changes its diagnostic filename.
The line number must be a positive integer representable by `int`. Macro operands
are expanded. Filenames may escape quotes and backslashes with a backslash.
Logical filenames do not change the directory used to resolve quoted includes.

Preprocessed output contains `#line` markers on entry to and return from each
include. The lexer consumes these markers so parser diagnostics and AST source
locations retain the header's filename and line, then resume the caller's
location. Preprocessor diagnostics also respect explicit `#line` directives.

### Lexing and Parsing

The reentrant Flex lexer and Bison parser turn source text into an abstract
syntax tree (AST). The [AST definitions](../src/ast_node.hpp) represent the
language constructs consumed by later compiler stages.

### Bytecode Generation

A visitor-pattern [bytecode generator](../src/code_generator.cpp) traverses the
AST and emits VM bytecode. See [vm-architecture.md](vm-architecture.md) for the
execution model and bytecode architecture.

### Compiler Frontend

`lpcfront` is the command-line compiler frontend and provides AST output for
inspecting parsed programs. Build and test instructions are in
[development.md](development.md).

## Ongoing Specification Work

As the implementation develops, expand this outline with declaration and scope
rules, type and conversion rules, preprocessing behavior, supported efuns, and
compiler diagnostics. Document implemented behavior alongside relevant parser,
compiler, and preprocessor tests; distinguish proposed features from supported
ones.
