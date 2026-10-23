COMPILER STRESS TEST SUITE

IMPORTANT LANGUAGE NOTE
The requested features mix C and C++. Files ending in .cpp.c are C++-style
tests stored with a .c suffix for convenience. Rename them to .cpp if your
compiler selects grammar by file extension. Do not compile them with a standard
C compiler. A custom compiler may support a subset of these features.

The expected-valid / expected-error split below is primarily for parser testing.
Some "error" tests are syntactically valid but should fail semantic analysis.
If your compiler currently implements parsing/AST construction only, it may
accept these semantic-error cases while still producing an AST; that is not
necessarily a parser bug.

Expected valid (syntax/AST tests):
01-15. Feature coverage: declarations, precedence, control flow, switch,
goto, functions, recursion, overloading, function pointers, arrays, pointers,
structs, enum, typedef, static, sizeof, classes, access modifiers, inheritance,
references, this, new/delete.

Expected to fail:
16-18 syntax errors: missing semicolon, unmatched parenthesis, missing brace.
19 duplicate case value: semantic error.
20 break outside loop/switch: semantic/context error.
21 continue outside loop: semantic/context error.
22 undefined goto label: semantic error.
23 duplicate label: semantic error.
24 wrong function argument count: semantic/type-checking error.
25 malformed function pointer declaration: syntax error.
26 zero-sized array: language/semantic constraint (some custom grammars may permit it).
27 pointer initialized with integer: type error.
28 unknown struct member: semantic error.
29 private member accessed outside class: access-control error.
30 reference without initializer: C++ semantic error.
31 delete applied to non-pointer: semantic/type error.
32 this used in a free function: semantic error.
33 conflicting typedef redeclaration: semantic error.
34 ambiguous overload: semantic error; exact ambiguity depends on your conversion rules.
35 unknown base class: semantic error.
36 incompatible return type: type error.

Testing procedure:
1. First run 01, 03, 06, 07, 10, 16, 17, 25.
2. For each valid input, verify that AST node types and parent-child nesting are correct.
3. For syntax errors, require a clear parser error and nonzero exit status if that is your convention.
4. For semantic errors, only expect rejection if semantic analysis is implemented. A parser-only AST builder can legitimately construct an AST for them.
5. If your grammar supports only C or only C++, skip files using unsupported language features. A compiler cannot treat classes, references, this, inheritance, new/delete, and function overloading as standard C features.
