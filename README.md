# Group Details

| S. No. | Name | Enrollment Number |
| --- | --- | --- |
| 1 | Aditya Yadav | 24114007 |
| 2 | Anurag Kishor Patil | 24114015 |
| 3 | Divyanshu Meena | 24114034 |
| 4 | Ravi Kumar | 24114075 |

# Intermediate Code Generator for C/C++ Subset

A C++ Flex lexer and Bison parser extended with a three-address-code (TAC)
intermediate-code generator for a practical educational C/C++ subset. The
existing lexer, parser, AST and semantic checks remain the front end; IR is
generated only after those stages accept the source program.

Supported features include `#include`, primitive types, user-defined type names, identifiers, literals, declarations, initializers, arrays, pointers, structures, classes and objects, simple `enum` declarations, function definitions, function calls with arguments, recursive-call syntax, `if/else`, `switch/case/default`, `while`, `do/while`, `for`, `goto` and statement labels, `break`, `continue`, member access, dynamic memory allocation using `new` and `delete`, comments, punctuation, and arithmetic, logical, relational, equality, bitwise, shift, assignment, and compound-assignment operators. The `static` storage-class specifier is preserved in the parsed declaration.

The grammar is implemented in `src/parser.y`. Lexical analysis is implemented in `src/lexer.l`. Supporting C++ source files are `src/main.cpp`, `src/tokens.cpp`, and `src/tokens.h`.

For a valid input program, the original invocation still prints a token table.
`--ast` still prints the AST. `--ir` writes TAC to the requested file. Invalid
input reports lexical, syntax, or semantic errors and no IR is produced.

The TAC contains temporary variables (`t1`, `t2`, ...), labels and branches,
assignments, `param`/`call`, `return`, array/member accesses, allocation and
deallocation. It covers declarations and initializers, expressions, functions,
calls, conditionals, loops, `switch`, `break`/`continue`, and `goto`/labels.
Type declarations are represented as comments because they do not execute.

## Build and Run

```sh
make clean
make
./syntax_analyzer test/operators.c
```

Generate an IR file:

```sh
./syntax_analyzer --ir output.ir test/ir_cases/02_control_flow.c
cat output.ir
```

## Run All Tests

```sh
./run.sh ./syntax_analyzer
```

or:

```sh
make test
```

`run.sh` writes IR files under `test/ir_output/` for every accepted `.c` case
under the test tree, including the supplied feature and stress tests. Invalid
test cases are expected to report an error and do not create IR files.

Run the parser/AST integration cases with:

```sh
make test-ir
```

Run targeted IR semantic regression checks (loop initialization, multidimensional
initializers, aliases, overload symbols, object receivers, enum/static metadata,
and ambiguous-overload rejection):

```sh
make test-ir-features
```

The cases are in `test/ir_cases/`. Cases 01-09 and 11-12 must parse
successfully; case 10 intentionally contains syntax and lexical errors. Their
expected AST signatures are documented in `test/ir_cases/EXPECTED.md`.

## Directory Structure

```text
src/
  lexer.l
  parser.y
  main.cpp
  ast.cpp / ast.hpp
  semantic.cpp / semantic.hpp
  ir.cpp / ir.hpp
  tokens.cpp
  tokens.h

test/
  ir_cases/                 # 12 integration cases
  compiler_stress_tests/    # valid and invalid feature cases
  *.c                       # additional language tests

Makefile
run.sh
README.md
```

Build requirements: G++, Bison, Flex, and Make.
