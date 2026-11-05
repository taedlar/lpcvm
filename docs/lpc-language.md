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
