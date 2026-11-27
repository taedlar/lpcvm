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

- Assignments evaluate the RHS before the LHS. Compound assignments read the
  target value after evaluating the RHS. Other binary operators evaluate the
  LHS first.
- `&&` and `||` return the last evaluated operand rather than a normalized 0 or 1.
- Mixed numeric operands are promoted to a common type: floating point takes
  precedence, and mixed-sign integers become `int64`, or `uint64` when the
  unsigned operand is 64-bit.
- String `+` concatenates two strings; string `+=` assigns the concatenated
  value to its target. Concatenation preserves embedded NUL bytes.

### Arrays

Array literals use `({value, ...})`, including `({})` for an empty array.
`array[index]` reads an element and can be used with `=`, compound assignments,
and prefix or postfix `++`/`--`. Indexes must be nonnegative integers smaller
than the array size; invalid types and out-of-range accesses raise errors.

Assignments evaluate the RHS first, then the array expression and index once,
in that order. Compound assignments read the element after those evaluations.
Arrays have reference semantics: writes are visible through aliases. Strings
stored in arrays have owned storage independent of local string buffers.

### Mappings

Mapping literals use `([key: value, ...])`, including `([])` for an empty mapping.
`mapping[key]` supports reads, assignment, compound assignment, and prefix or
postfix `++`/`--`, with the same evaluation order as array indexing. Reads of
missing keys return an empty value (`std::monostate`) without inserting a key;
assignment inserts or replaces the value. Compound operations use the ordinary
operator rules for the loaded value, including empty values.

Keys follow mapping equality rules: numeric types remain distinct, strings
compare by content, and arrays and mappings compare by identity. Stored string
keys and values own their storage. Mappings have reference semantics, so writes
are visible through aliases. Indexed expressions can be nested.

### Statements and Control Flow

The current statement set covers blocks with local variables, `if`/`else`,
`while`, `do`/`while`, `for`, `break`, `continue`, and `return`.

### Resource descriptors and module calls

LPCVM separates naming a dependency from acquiring and calling it. A resource
descriptor is a compile-time name for a binding, not an ordinary LPC value: it
cannot be assigned, passed as an argument, or stored in a variable. A mount
alias can only be established by a `mount` declaration. This makes resource
dependencies visible to code inspection and policy checks.

`call_other` targets an existing object; it does not implicitly load an LPC
blueprint or invoke the compiler. Loading and binding resources are explicit.

An aliased mount declares the name that LPC code uses:

```
mount ("/daemon/channel" => Channel);

using (Channel) {
    ::chat("hello");
}
```

A function-local mount can defer binding until runtime, allowing policy to
inspect the current instance:

```
int send(string requested_channel) {
    mount (requested_channel => Channel);
    using (Channel) {
        ::chat("hello");
    }
}
```

An explicit `using(Alias)` acquires the selected resource for its block, subject
to policy, and establishes the module context for non-prefixed `::` calls.
`::method()` calls a method on that selected module. It does not search other
mounts or fall through to the global EFUN context when the selected module lacks
the method. Nested `using` blocks select the innermost context. The context is
lexical and is not inherited by functions called from inside the block.

Unaliased global mounts provide the implicit root EFUN namespace:

```
mount ("efun://console");

int example() {
    ::write("hello");
    return 1;
}
```

The root namespace can combine methods from multiple unaliased global mounts.
Two providers for the same referenced method are ambiguous and are rejected.
An explicit `using` context selects one module instead; it does not merge that
module with the root namespace. An explicit EFUN-bypass syntax, if provided, is
a separate language and policy decision.

Local unaliased mounts are rejected by design. A local mount must have an alias
so `using` can select one policy-mediated binding rather than search an
unspecified set of resources. Function-local mount paths may be dynamic. At
runtime, policy can inspect the requested identity and current object instance,
then bind the alias to the requested resource or to a compatible LPC wrapper
that preserves the declared method interface while applying additional
restrictions. Global dependency decisions are made while loading the blueprint
and belong to that class generation; local
decisions are made when the mount executes and belong to that execution frame.
Hot reload therefore re-evaluates class-owned bindings, while local bindings
can reflect per-instance state.

Inheritance aliases are object-scoped descriptors, similar to mounted-module
aliases, and are not ordinary values either. An inherited function can be
called by name through normal method resolution or explicitly through an
inheritance alias followed by `.` (for example, `Parent.update()`). `::` is
reserved for selecting the active module context, not for inherited calls.
Each direct inheritance receives a zero-based inheriting slot in declaration
order, whether or not it has an alias. An alias refers to that inheriting slot.
These slots are independent of global and function-local mount slots;
inheritance does not consume a resource slot.

### Current resource implementation

The current compiler requires global `mount` paths to be compile-time strings:
string literals (including adjacent literals and macro-expanded literals) or
`+` concatenations of those strings. The parser folds these concatenations into
string literal AST nodes.
Aliased global mounts currently resolve registered native modules during code
generation and retain them in the class; missing or non-native modules are errors.
Unaliased global mounts also resolve registered native modules. Calls such as
`::method()` search their exported methods during code generation and import only
the referenced methods into a class-owned `NativeModule` in mount slot 0. Repeated
calls reuse the same import. Missing methods are compile errors; if multiple
unaliased mounts export a referenced name, code generation rejects the ambiguity.
All global mounts are collected before functions are generated, so their position
relative to function definitions does not affect this search. Unreferenced exports
are not imported and do not cause duplicate-name errors.
Function-local aliased mounts may use dynamic path expressions. Their current
binding stores the resource name, and direct `Alias::method()` calls look up
the native module when called. `using(Alias)` resolves the alias on entry,
requires a `NativeModule`, and holds a strong reference until the context exits.
Unprefixed `::method()` calls in that context dispatch only through the held
module; they do not fall back to the root EFUN namespace. Local unaliased mounts
are not supported. Runtime policy substitution, LPC-module dispatch through a
mount, and inheritance/object `using` contexts are not implemented.

Mount slots are separate from variable slots. Class slot 0 is reserved for the
anonymous (implicit) mount. Named `mount` and `inherit` aliases receive slots in
their symbol entries. Unaliased `inherit` and `mount` declarations retain a
string literal in the AST without creating an alias or allocating an alias slot.
Class aliases occupy persistent slots. Function-local aliases start at local
slot 0 in a separate overlay relative to the call-frame mount base, reusing
slots when their enclosing block or loop scope ends. Each method binding records
its peak local mount count. The class mount count includes only slot 0 and class
aliases.

### LPC packages and modules

We borrow the concepts of **package** and **module** from Python to allow LPC code to
organize the mudlib with better control.

A "module" is a LPC source file that can be loaded to serve specific purpose. LPC code
cal call methods in a module (blueprint instance), or clone the module to create new objects.

A "package" is a filesystem directory with a `init.lpc` file. LPC code can also mount a
package using a path end with `/`, and clone new objects in the package. Package itself
cannot be cloned.

LPC modules and packages can be represented with resource descriptors in LPCVM.

Simul-efuns can be implemented as modules or packages. An explicit `using`
context provides a scoped, policy-inspectable simul-efun selection rather than
an ambient object-wide override.

### EFUN packages

In LPCVM, EFUNs are provided as modules. Unaliased global mounts contribute to
the implicit root EFUN namespace; an explicit `using` context selects a
policy-bound module for `::` calls within its block.

### Inheritance

Inheritance lets an object blueprint acquire state and behavior from other
blueprints. The following rules define the intended language design; inheritance
loading, lookup, and dispatch are not yet fully implemented by the compiler.

#### Declarations and policy binding

Inheritance declarations are object-scoped and name a compile-time string path,
with an optional alias:

```lpc
inherit "/std/base";
private inherit "/std/helper" => Helper;
```

Like global mounts, inheritance requests can pass through policy inspection
while loading the blueprint. Policy may reject a request or substitute a
different blueprint identity. The selected blueprint is resolved before method
lookup and state layout; its actual interface is used to check calls. The
binding belongs to that class generation, is retained for its lifetime, and
is re-evaluated when loading a new generation. It cannot vary per call or per
instance of the same generation.

Every declaration creates a distinct direct inheritance slot and state subtree,
even if several requests select the same actual blueprint. An alias names its
declaration's slot, not the requested or selected blueprint identity. Aliases
are lexical to the declaring blueprint and are not inherited as names.
Inherited method bodies retain their original alias bindings.

Cyclic inheritance is rejected using the actual blueprint identities selected
after policy inspection along each ancestry path. Repeated siblings and diamonds
are allowed; neither is a cycle. Substitution cannot bypass cycle detection.

#### Static method lookup

Calls without an explicit object receiver are resolved at compile time:

| Call | Resolution |
| --- | --- |
| `f()` | Local method first, then accessible inherited methods |
| `Parent.f()` | Accessible methods through the direct slot named by `Parent` |
| `obj->f()` | Public virtual table of the receiving object at runtime |
| `self->f()` | Public virtual table of the actual receiving object at runtime |

For `f()` compiled in blueprint `X`, first look for a method declared in `X`.
Otherwise, search its direct parents in reverse declaration order. Search each
parent's exposed methods recursively, depth-first, before proceeding to the
next parent. A parent's own public method precedes methods exposed through its
public inheritance edges. Inaccessible methods are excluded from the search.
All declarations participate regardless of their position in the source file.

For example, if `D` inherits `B`, then `C`, and `C` publicly inherits `A.f`,
lookup selects `A.f` through `C` before considering `B.f`, even if `B` defines
`f` directly. Lookup is by name, not argument types or arity. After selecting a
method, an invalid call is an error; lookup does not retry another parent.

`Parent.f()` applies the same recursive search to the selected parent's exposed
methods, bypassing local methods and all sibling inheritance paths. It cannot
access a private method of that parent. There is no implicit `.f()` shorthand;
use an explicit alias to select a parent. `::` remains module-call syntax.

#### Visibility and private inheritance

Methods and inheritance edges are public by default. Instance variables are
private by default. Explicit visibility replaces the default; contradictory
visibility modifiers are errors.

A private method is callable only by static calls compiled in its defining
blueprint. It is unavailable to descendant lookup and never enters the public
virtual table. A private local method declaration that collides with a public
method exposed by a publicly inherited parent is an error: a child cannot reduce
the visibility of an inherited public method.

A private inheritance edge still makes the parent's exposed methods available
for static calls inside the blueprint declaring that edge, both by name and
through its alias. The edge does not expose those methods to descendants or
contribute them to that blueprint's public virtual table. Existing static calls
inside inherited method bodies remain valid across this boundary.

For example, if `B` privately inherits `A`, a public `B.run()` may statically call
`A.f`. A descendant can call `run`, but cannot directly reach `A.f` through that
private edge. A public wrapper is the explicit way to expose selected behavior.

#### Dynamic dispatch and binding lifetime

Every `->` call uses the receiving object's public virtual table, consistent
with the public method interface used for native modules. `self` grants no
additional access: `self->f()` cannot call a private method. A missing public
method is a dynamic lookup error, with no fallback to private methods or module
contexts.

For each name, the public virtual table uses the same local-first,
reverse-parent, depth-first precedence, considering only public methods and
crossing only public inheritance edges. Each selected entry identifies both
the implementation and its inheritance path.

Static calls retain the binding established when their defining blueprint was
compiled; inheriting that blueprint does not rebind its method bodies. `self`
always denotes the complete receiving object, including inside inherited methods:

```lpc
// A
int f() { return 1; }
int run() { return f(); }
int dynamic_run() { return self->f(); }

// D (a separate source file)
inherit "A";
int f() { return 2; }
```

On an instance of `D`, `run()` returns `1`, while `dynamic_run()` returns `2`.

#### Replicated state

Inheritance replicates state. If `D` inherits `B` and `C`, both of which inherit
`A`, the two paths have independent copies of `A`'s instance variables. Repeated
direct inheritance likewise creates independent copies; unqualified lookup
favors the last declaration, and aliases can select each copy explicitly.

A static binding identifies an implementation and its relative inheritance
path. Execution preserves that path when the defining blueprint is inherited,
so a static call through `B` uses `B`'s copy of `A`'s state. A dynamic call uses
the state path selected by the receiver's virtual table, which may differ from
the calling method's path. In either case, `self` still denotes the complete
receiver.

### Composition models

LPC supports two complementary composition models: assembling components into
a blueprint through inheritance, and connecting separate objects through object
members. A virtual-world object can use both models together.

**Blueprint composition** uses private replicated inheritance and aliases to
assemble state and behavior when a blueprint is loaded. Each inheritance slot
provides independent component state, while aliases select component methods
at compile time. Private inheritance keeps those methods out of the host's
public interface unless the host exposes them through public wrappers.

```lpc
private inherit "/components/counter" => Hits;
private inherit "/components/counter" => Misses;

void record_hit() { Hits.increment(); }
void record_miss() { Misses.increment(); }
int hit_count() { return Hits.value(); }
```

The two counters have separate state but share the containing object's identity
and lifetime. Inside either component, `self` denotes the host. An ordinary
internal call remains statically bound; `self->method()` deliberately calls the
host's public virtual table. Components are fixed for the class generation and
cannot be independently shared or replaced per instance.

Blueprint composition also makes policy inspection easier at compile time,
during blueprint loading. Explicit inheritance declarations expose the requested
dependencies before method binding and state layout. Through the loading policy,
administrators can inspect each dependency, reject disallowed blueprints, or
substitute approved implementations, including for transitive inheritance
requests. Compilation then checks calls against the selected implementations
and retains those bindings for the class generation. This gives administrators
control over which component implementations untrusted code can incorporate.
Private inheritance itself is an encapsulation mechanism, not a separate
security boundary; runtime resource access and dynamic object calls remain
subject to their applicable policies.

**Object composition** stores references to separate objects in object members.
Calls through those references use `->` and the component object's public
virtual table. Each component has its own identity: `self` inside it denotes
that component rather than its owner. References allow components to be shared
between owners or replaced per instance, and component lifetimes are not
intrinsically tied to one owner. Acquiring and using these objects remains
subject to runtime policy; storing a reference does not implicitly load or
compile a blueprint.

Use blueprint composition for embedded components with a shared identity and
lifetime, and object composition when separate identity, sharing, or runtime
replacement is needed. For example, a world object may embed counters through
private inheritance while holding object references to its location and owner.

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

`LpcCompiler::generate_code()` enables accumulator lowering by default
for addition chains made entirely of string literals and statically declared
scalar string variables. Parenthesized string chains are flattened in operand
order. Other expressions retain ordinary lowering, though eligible subchains
can still be optimized independently. Calls and side-effecting expressions are
never evaluated with an accumulator active. Temporary results stored in scalar `string` locals become fabric-pooled buffers
by default; shared-string and buffer sources retain their references. Eligible
`=`/`+=` chains finish directly into the local's buffer, so executing such code
requires a fabric.

The `use_string_accumulator` and `use_local_string_buffer` fields of
`lpcvm::CodeGenerationOptions` select ordinary lowering for benchmarks and
comparison. See [string-handling.md](string-handling.md) for assignment costs and LPC
programming guidance, and [string-runtime.md](string-runtime.md) for the complete
storage and accumulator design.

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
