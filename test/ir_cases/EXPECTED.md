# Parser and AST integration cases

Run a valid case with:

    ./syntax_analyzer --ast test/ir_cases/01_expressions.c

Cases 01-09 and 11-12 must exit with status **0**. Case 10 must exit with status **1**.
The AST names below are the expected output signatures; extra List, Pointer, and
Declarator nodes are normal.

| Case | Purpose | Expected result |
| --- | --- | --- |
| 01_expressions.c | Operator precedence, grouping, logical/bitwise/shift operations, compound assignment | Exit 0; AST contains Binary(*), Binary(<<), Assignment(+=), Binary(||), and Unary(!). |
| 02_control_flow.c | while, do-while, for, nested if, break, continue, postfix operators | Exit 0; AST contains While, DoWhile, For, If, Break, Continue, and Postfix(++). |
| 03_switch_cases.c | Multiple case labels, default, function call and unary minus | Exit 0; AST contains Switch, three Case nodes, Default, Call, and Unary(-). |
| 04_recursion_calls.c | Recursive function calls and return expressions | Exit 0; AST contains FunctionDefinition(fibonacci), two nested Call nodes, If, and Return. |
| 05_arrays_pointers.c | Multi-dimensional arrays, nested initializers, string array, pointers, address-of, dereference | Exit 0; AST contains ArrayDeclarator, InitializerList, ArrayAccess, Unary(&), and Unary(*). |
| 06_struct_class_members.c | Structures, classes, access specifiers, object declarations, member access | Exit 0; AST contains StructDefinition(Pair), ClassDefinition(Vault), AccessSpecifier(public), AccessSpecifier(private), AccessSpecifier(protected), and MemberAccess. |
| 07_enum_static_goto.c | Enumerators with an explicit value, static function, source label, and goto | Exit 0; AST contains EnumDefinition, Enumerator(RUNNING), StaticType, Label(retry), and Goto(retry). |
| 08_dynamic_function_pointer.c | Function-pointer declarator/call, new[], delete[], array access | Exit 0; AST contains GroupedDeclarator, Call, New, Delete, ArrayDelete, and ArrayAccess. |
| 09_preprocessor_literals.c | #include, comments, boolean/character/float/string literals, adjacent strings, escaped characters | Exit 0; AST contains Include, BooleanLiteral(true), CharLiteral, FloatLiteral(1.5e+2), and StringLiteral. |
| 10_invalid_syntax_and_lexing.c | Error recovery for malformed expression and illegal character | Exit 1; diagnostics include a syntax error near ; after + and Lexical error ... Illegal character '@'. No successful AST output is expected. |
| 11_inheritance_typedef_reference_this_sizeof.c | Inheritance, typedef aliases, l-value references, this pointer, and sizeof | Exit 0; AST contains ClassDefinition(Child), BaseClass(Parent), TypedefType, Reference(&), This, MemberAccess(->inherited), SizeofType, and Sizeof. |
| 12_overloading_multilevel_pointers.c | Multiple functions with the same name and three pointer levels | Exit 0; AST contains two FunctionDefinition(blend) nodes, Pointer(***), Unary(&), Unary(*), and Call nodes for both overload-shaped calls. |
