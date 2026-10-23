%define parse.error detailed
%glr-parser
// C++ lets `Type object(args)` mean direct initialization while `Type name(...)`
// may introduce a function.  The GLR parser preserves both interpretations
// until later tokens choose one; constructor declarations add one similar case.
%expect 4
// `IDENTIFIER` is deliberately accepted as a user-defined type name.  In a
// GLR parser that makes `Type &name` temporarily ambiguous with `value & expr`;
// semantic analysis will resolve type names when symbol tables are added.
%expect-rr 3
%locations
%code requires {
#include <string>
#include "ast.hpp"
}
%code {
#include <cstdio>
#include <string>

int yylex();
void yyerror(const char *message);

static std::string take_text(std::string *text)
{
   std::string value = text ? *text : "";
   delete text;
   return value;
}
}
%union {
   AstNode *node;
   std::string *text;
}
%token INCLUDE_DIRECTIVE
%token INT FLOAT DOUBLE CHAR BOOL VOID UNSIGNED IF ELSE WHILE FOR DO SWITCH CASE DEFAULT RETURN BREAK CONTINUE GOTO
%token STRUCT CLASS PUBLIC PRIVATE PROTECTED NEW DELETE
%token STATIC ENUM TYPEDEF THIS SIZEOF
%token TRUE FALSE
%token <text> IDENTIFIER LABEL INTEGER_LITERAL FLOAT_LITERAL CHAR_LITERAL STRING_LITERAL
%token PLUS MINUS STAR SLASH PERCENT ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN PERCENT_ASSIGN
%token SHL_ASSIGN SHR_ASSIGN BIT_AND_ASSIGN BIT_OR_ASSIGN BIT_XOR_ASSIGN
%token EQ NE LT LE GT GE AND OR NOT INC DEC BIT_AND BIT_OR BIT_XOR BIT_NOT SHL SHR
%token LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET SEMICOLON COMMA DOT ARROW SCOPE COLON QUESTION INVALID
%type <node> program external_list external_declaration type_specifier declaration declaration_specifier
%type <node> init_declarator_list init_declarator initializer initializer_list declarator pointer_opt direct_declarator
%type <node> function_definition constructor_definition constructor_initializer_opt constructor_initializer_list constructor_initializer
%type <node> parameter_list_opt parameter_list parameter_declaration struct_definition class_definition
%type <node> inheritance_opt base_specifier_list base_specifier
%type <node> enum_definition enum_name_opt enumerator_list_opt enumerator_list enumerator member_list_opt member_list
%type <node> member_declaration class_member_list_opt class_member_list class_member access_specifier compound_statement
%type <node> block_items block_item statement expression_statement selection_statement switch_sections_opt switch_sections
%type <node> switch_section switch_label constant_expression iteration_statement for_init expression_opt
%type <node> jump_statement expression assignment_expression conditional_expression logical_or_expression logical_and_expression
%type <node> bitwise_or_expression bitwise_xor_expression bitwise_and_expression equality_expression relational_expression
%type <node> shift_expression additive_expression multiplicative_expression unary_expression new_declarator_opt new_initializer_opt
%type <node> delete_array_opt postfix_expression argument_list_opt argument_list primary_expression string_literal_seq
%type <node> sizeof_type_specifier
%right ASSIGN PLUS_ASSIGN MINUS_ASSIGN STAR_ASSIGN SLASH_ASSIGN PERCENT_ASSIGN
%right SHL_ASSIGN SHR_ASSIGN BIT_AND_ASSIGN BIT_OR_ASSIGN BIT_XOR_ASSIGN
%right QUESTION COLON
%left OR
%left AND
%left BIT_OR
%left BIT_XOR
%left BIT_AND
%left EQ NE
%left LT LE GT GE
%left SHL SHR
%left PLUS MINUS
%left STAR SLASH PERCENT
%right NOT BIT_NOT UMINUS UPLUS DEREF ADDRESS
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%start program
%%
program:
   external_list { ast_set_root(make_ast_node("Program", "", {$1})); };
external_list:
   %empty { $$ = make_ast_list(); }
 | external_list external_declaration { $$ = append_ast_list($1, $2); };
external_declaration:
   INCLUDE_DIRECTIVE { $$ = make_ast_node("Include"); }
 | declaration { $$ = $1; }
 | function_definition { $$ = $1; }
 | struct_definition { $$ = $1; }
 | class_definition { $$ = $1; }
 | enum_definition { $$ = $1; }
 | error SEMICOLON { $$ = make_ast_node("Error"); };
type_specifier:
   INT { $$ = make_ast_node("Type", "int"); }
 | UNSIGNED INT { $$ = make_ast_node("Type", "unsigned int"); }
 | FLOAT { $$ = make_ast_node("Type", "float"); }
 | DOUBLE { $$ = make_ast_node("Type", "double"); }
 | CHAR { $$ = make_ast_node("Type", "char"); }
 | BOOL { $$ = make_ast_node("Type", "bool"); }
 | VOID { $$ = make_ast_node("Type", "void"); }
 | IDENTIFIER { $$ = make_ast_node("Type", take_text($1)); }
 | STRUCT IDENTIFIER { $$ = make_ast_node("Type", "struct " + take_text($2)); }
 | CLASS IDENTIFIER { $$ = make_ast_node("Type", "class " + take_text($2)); }
 | ENUM IDENTIFIER { $$ = make_ast_node("Type", "enum " + take_text($2)); };
declaration:
   declaration_specifier init_declarator_list SEMICOLON { $$ = make_ast_node("Declaration", "", {$1, $2}); }
 | declaration_specifier init_declarator_list error { $$ = make_ast_node("Declaration", "", {$1, $2}); };
declaration_specifier:
   type_specifier { $$ = $1; }
 | STATIC type_specifier { $$ = make_ast_node("StaticType", "", {$2}); }
 | TYPEDEF type_specifier { $$ = make_ast_node("TypedefType", "", {$2}); };
init_declarator_list:
   init_declarator { $$ = append_ast_list(make_ast_list(), $1); }
 | init_declarator_list COMMA init_declarator { $$ = append_ast_list($1, $3); };
init_declarator:
   declarator { $$ = make_ast_node("InitDeclarator", "", {$1}); }
 | declarator ASSIGN initializer { $$ = make_ast_node("InitDeclarator", "", {$1, $3}); }
 | declarator LPAREN argument_list_opt RPAREN { $$ = make_ast_node("DirectInitializer", "", {$1, $3}); };
initializer:
   assignment_expression { $$ = $1; }
 | LBRACE RBRACE { $$ = make_ast_node("InitializerList"); }
 | LBRACE initializer_list RBRACE { $$ = make_ast_node("InitializerList", "", {$2}); }
 | LBRACE initializer_list COMMA RBRACE { $$ = make_ast_node("InitializerList", "", {$2}); };
initializer_list:
   initializer { $$ = append_ast_list(make_ast_list(), $1); }
 | initializer_list COMMA initializer { $$ = append_ast_list($1, $3); };
declarator:
   pointer_opt direct_declarator { $$ = make_ast_node("Declarator", "", {$1, make_ast_node("Reference", ""), $2}); }
 | pointer_opt BIT_AND direct_declarator { $$ = make_ast_node("Declarator", "", {$1, make_ast_node("Reference", "&"), $3}); };
pointer_opt:
   %empty { $$ = make_ast_node("Pointer", ""); }
 | pointer_opt STAR { $1->value += "*"; $$ = $1; };
direct_declarator:
   IDENTIFIER { $$ = make_ast_node("Identifier", take_text($1)); }
 | LPAREN STAR pointer_opt direct_declarator RPAREN { $3->value += "*"; $$ = make_ast_node("GroupedDeclarator", "", {$3, $4}); }
 | direct_declarator LBRACKET expression_opt RBRACKET { $$ = make_ast_node("ArrayDeclarator", "", {$1, $3}); }
 | direct_declarator LPAREN parameter_list_opt RPAREN { $$ = make_ast_node("FunctionDeclarator", "", {$1, $3}); };
function_definition:
   declaration_specifier IDENTIFIER LPAREN parameter_list_opt RPAREN compound_statement
   { $$ = make_ast_node("FunctionDefinition", take_text($2), {$1, $4, $6}); };
constructor_definition:
   IDENTIFIER LPAREN parameter_list_opt RPAREN constructor_initializer_opt compound_statement
   { $$ = make_ast_node("ConstructorDefinition", take_text($1), {$3, $5, $6}); };
constructor_initializer_opt:
   %empty { $$ = make_ast_list(); }
 | COLON constructor_initializer_list { $$ = $2; };
constructor_initializer_list:
   constructor_initializer { $$ = append_ast_list(make_ast_list(), $1); }
 | constructor_initializer_list COMMA constructor_initializer { $$ = append_ast_list($1, $3); };
constructor_initializer:
   IDENTIFIER LPAREN argument_list_opt RPAREN
   { $$ = make_ast_node("ConstructorInitializer", take_text($1), {$3}); };
parameter_list_opt:
   %empty { $$ = make_ast_list(); }
 | parameter_list { $$ = $1; };
parameter_list:
   parameter_declaration { $$ = append_ast_list(make_ast_list(), $1); }
 | parameter_list COMMA parameter_declaration { $$ = append_ast_list($1, $3); };
parameter_declaration:
   type_specifier declarator { $$ = make_ast_node("Parameter", "", {$1, $2}); }
 | type_specifier { $$ = make_ast_node("Parameter", "", {$1}); };
struct_definition:
   STRUCT IDENTIFIER LBRACE member_list_opt RBRACE SEMICOLON
   { $$ = make_ast_node("StructDefinition", take_text($2), {$4}); };
class_definition:
   CLASS IDENTIFIER inheritance_opt LBRACE class_member_list_opt RBRACE SEMICOLON
   { $$ = make_ast_node("ClassDefinition", take_text($2), {$3, $5}); };
inheritance_opt:
   %empty { $$ = make_ast_list(); }
 | COLON base_specifier_list { $$ = $2; };
base_specifier_list:
   base_specifier { $$ = append_ast_list(make_ast_list(), $1); }
 | base_specifier_list COMMA base_specifier { $$ = append_ast_list($1, $3); };
base_specifier:
   IDENTIFIER { $$ = make_ast_node("BaseClass", take_text($1)); }
 | access_specifier IDENTIFIER { $$ = make_ast_node("BaseClass", take_text($2), {$1}); };
enum_definition:
   ENUM enum_name_opt LBRACE enumerator_list_opt RBRACE SEMICOLON
   { $$ = make_ast_node("EnumDefinition", "", {$2, $4}); };
enum_name_opt:
   %empty { $$ = make_ast_node("Identifier", ""); }
 | IDENTIFIER { $$ = make_ast_node("Identifier", take_text($1)); };
enumerator_list_opt:
   %empty { $$ = make_ast_list(); }
 | enumerator_list { $$ = $1; }
 | enumerator_list COMMA { $$ = $1; };
enumerator_list:
   enumerator { $$ = append_ast_list(make_ast_list(), $1); }
 | enumerator_list COMMA enumerator { $$ = append_ast_list($1, $3); };
enumerator:
   IDENTIFIER { $$ = make_ast_node("Enumerator", take_text($1)); }
 | IDENTIFIER ASSIGN constant_expression { $$ = make_ast_node("Enumerator", take_text($1), {$3}); };
member_list_opt:
   %empty { $$ = make_ast_list(); }
 | member_list { $$ = $1; };
member_list:
   member_declaration { $$ = append_ast_list(make_ast_list(), $1); }
 | member_list member_declaration { $$ = append_ast_list($1, $2); };
member_declaration:
   declaration { $$ = $1; }
 | function_definition { $$ = $1; }
 | constructor_definition { $$ = $1; };
class_member_list_opt:
   %empty { $$ = make_ast_list(); }
 | class_member_list { $$ = $1; };
class_member_list:
   class_member { $$ = append_ast_list(make_ast_list(), $1); }
 | class_member_list class_member { $$ = append_ast_list($1, $2); };
class_member:
   access_specifier COLON { $$ = $1; }
 | member_declaration { $$ = $1; };
access_specifier:
   PUBLIC { $$ = make_ast_node("AccessSpecifier", "public"); }
 | PRIVATE { $$ = make_ast_node("AccessSpecifier", "private"); }
 | PROTECTED { $$ = make_ast_node("AccessSpecifier", "protected"); };
compound_statement:
   LBRACE block_items RBRACE { $$ = make_ast_node("Block", "", {$2}); };
block_items:
   %empty { $$ = make_ast_list(); }
 | block_items block_item { $$ = append_ast_list($1, $2); };
block_item:
   declaration %dprec 2 { $$ = $1; }
 | enum_definition %dprec 2 { $$ = $1; }
 | statement %dprec 1 { $$ = $1; };
statement:
   compound_statement { $$ = $1; }
 | expression_statement { $$ = $1; }
 | selection_statement { $$ = $1; }
 | iteration_statement { $$ = $1; }
 | jump_statement { $$ = $1; }
 | LABEL statement { $$ = make_ast_node("Label", take_text($1), {$2}); }
 | error SEMICOLON { $$ = make_ast_node("Error"); };
expression_statement:
   SEMICOLON { $$ = make_ast_node("EmptyStatement"); }
 | expression SEMICOLON { $$ = make_ast_node("ExpressionStatement", "", {$1}); }
 | expression error { $$ = make_ast_node("ExpressionStatement", "", {$1}); };
selection_statement:
   IF LPAREN expression RPAREN statement %prec LOWER_THAN_ELSE { $$ = make_ast_node("If", "", {$3, $5}); }
 | IF LPAREN expression RPAREN statement ELSE statement { $$ = make_ast_node("If", "", {$3, $5, $7}); }
 | SWITCH LPAREN expression RPAREN LBRACE switch_sections_opt RBRACE { $$ = make_ast_node("Switch", "", {$3, $6}); };
switch_sections_opt:
   %empty { $$ = make_ast_list(); }
 | switch_sections { $$ = $1; };
switch_sections:
   switch_section { $$ = append_ast_list(make_ast_list(), $1); }
 | switch_sections switch_section { $$ = append_ast_list($1, $2); };
switch_section:
   switch_label block_items { $$ = make_ast_node("SwitchSection", "", {$1, $2}); };
switch_label:
   CASE constant_expression COLON { $$ = make_ast_node("Case", "", {$2}); }
 | DEFAULT COLON { $$ = make_ast_node("Default"); };
constant_expression:
   logical_or_expression { $$ = $1; };
iteration_statement:
   WHILE LPAREN expression RPAREN statement { $$ = make_ast_node("While", "", {$3, $5}); }
 | DO statement WHILE LPAREN expression RPAREN SEMICOLON { $$ = make_ast_node("DoWhile", "", {$2, $5}); }
 | FOR LPAREN for_init SEMICOLON expression_opt SEMICOLON expression_opt RPAREN statement
   { $$ = make_ast_node("For", "", {$3, $5, $7, $9}); };
for_init:
   %empty { $$ = make_ast_node("Empty"); }
 | expression { $$ = $1; }
 | type_specifier init_declarator_list { $$ = make_ast_node("Declaration", "", {$1, $2}); };
expression_opt:
   %empty { $$ = make_ast_node("Empty"); }
 | expression { $$ = $1; };
jump_statement:
   RETURN expression_opt SEMICOLON { $$ = make_ast_node("Return", "", {$2}); }
 | RETURN expression_opt error { $$ = make_ast_node("Return", "", {$2}); }
 | BREAK SEMICOLON { $$ = make_ast_node("Break"); }
 | BREAK error { $$ = make_ast_node("Break"); }
 | CONTINUE SEMICOLON { $$ = make_ast_node("Continue"); }
 | CONTINUE error { $$ = make_ast_node("Continue"); }
 | GOTO IDENTIFIER SEMICOLON { $$ = make_ast_node("Goto", take_text($2)); }
 | GOTO IDENTIFIER error { $$ = make_ast_node("Goto", take_text($2)); };
expression:
   assignment_expression { $$ = $1; }
 | expression COMMA assignment_expression { $$ = make_ast_node("Comma", "", {$1, $3}); };
assignment_expression:
   conditional_expression { $$ = $1; }
 | unary_expression ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "=", {$1, $3}); }
 | unary_expression PLUS_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "+=", {$1, $3}); }
 | unary_expression MINUS_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "-=", {$1, $3}); }
 | unary_expression STAR_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "*=", {$1, $3}); }
 | unary_expression SLASH_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "/=", {$1, $3}); }
 | unary_expression PERCENT_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "%=", {$1, $3}); }
 | unary_expression SHL_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "<<=", {$1, $3}); }
 | unary_expression SHR_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", ">>=", {$1, $3}); }
 | unary_expression BIT_AND_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "&=", {$1, $3}); }
 | unary_expression BIT_OR_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "|=", {$1, $3}); }
 | unary_expression BIT_XOR_ASSIGN assignment_expression { $$ = make_ast_node("Assignment", "^=", {$1, $3}); };
conditional_expression:
   logical_or_expression { $$ = $1; }
 | logical_or_expression QUESTION expression COLON conditional_expression
   { $$ = make_ast_node("Conditional", "", {$1, $3, $5}); };
logical_or_expression:
   logical_and_expression { $$ = $1; }
 | logical_or_expression OR logical_and_expression { $$ = make_ast_node("Binary", "||", {$1, $3}); };
logical_and_expression:
   bitwise_or_expression { $$ = $1; }
 | logical_and_expression AND bitwise_or_expression { $$ = make_ast_node("Binary", "&&", {$1, $3}); };
bitwise_or_expression:
   bitwise_xor_expression { $$ = $1; }
 | bitwise_or_expression BIT_OR bitwise_xor_expression { $$ = make_ast_node("Binary", "|", {$1, $3}); };
bitwise_xor_expression:
   bitwise_and_expression { $$ = $1; }
 | bitwise_xor_expression BIT_XOR bitwise_and_expression { $$ = make_ast_node("Binary", "^", {$1, $3}); };
bitwise_and_expression:
   equality_expression { $$ = $1; }
 | bitwise_and_expression BIT_AND equality_expression { $$ = make_ast_node("Binary", "&", {$1, $3}); };
equality_expression:
   relational_expression { $$ = $1; }
 | equality_expression EQ relational_expression { $$ = make_ast_node("Binary", "==", {$1, $3}); }
 | equality_expression NE relational_expression { $$ = make_ast_node("Binary", "!=", {$1, $3}); };
relational_expression:
   shift_expression { $$ = $1; }
 | relational_expression LT shift_expression { $$ = make_ast_node("Binary", "<", {$1, $3}); }
 | relational_expression LE shift_expression { $$ = make_ast_node("Binary", "<=", {$1, $3}); }
 | relational_expression GT shift_expression { $$ = make_ast_node("Binary", ">", {$1, $3}); }
 | relational_expression GE shift_expression { $$ = make_ast_node("Binary", ">=", {$1, $3}); };
shift_expression:
   additive_expression { $$ = $1; }
 | shift_expression SHL additive_expression { $$ = make_ast_node("Binary", "<<", {$1, $3}); }
 | shift_expression SHR additive_expression { $$ = make_ast_node("Binary", ">>", {$1, $3}); };
additive_expression:
   multiplicative_expression { $$ = $1; }
 | additive_expression PLUS multiplicative_expression { $$ = make_ast_node("Binary", "+", {$1, $3}); }
 | additive_expression MINUS multiplicative_expression { $$ = make_ast_node("Binary", "-", {$1, $3}); };
multiplicative_expression:
   unary_expression { $$ = $1; }
 | multiplicative_expression STAR unary_expression { $$ = make_ast_node("Binary", "*", {$1, $3}); }
 | multiplicative_expression SLASH unary_expression { $$ = make_ast_node("Binary", "/", {$1, $3}); }
 | multiplicative_expression PERCENT unary_expression { $$ = make_ast_node("Binary", "%", {$1, $3}); };
unary_expression:
   postfix_expression { $$ = $1; }
 | INC unary_expression { $$ = make_ast_node("Prefix", "++", {$2}); }
 | DEC unary_expression { $$ = make_ast_node("Prefix", "--", {$2}); }
 | PLUS unary_expression %prec UPLUS { $$ = make_ast_node("Unary", "+", {$2}); }
 | MINUS unary_expression %prec UMINUS { $$ = make_ast_node("Unary", "-", {$2}); }
 | NOT unary_expression { $$ = make_ast_node("Unary", "!", {$2}); }
 | BIT_NOT unary_expression { $$ = make_ast_node("Unary", "~", {$2}); }
 | STAR unary_expression %prec DEREF { $$ = make_ast_node("Unary", "*", {$2}); }
 | BIT_AND unary_expression %prec ADDRESS { $$ = make_ast_node("Unary", "&", {$2}); }
 | NEW type_specifier new_declarator_opt new_initializer_opt { $$ = make_ast_node("New", "", {$2, $3, $4}); }
 | DELETE delete_array_opt unary_expression { $$ = make_ast_node("Delete", "", {$2, $3}); }
 | SIZEOF unary_expression { $$ = make_ast_node("Sizeof", "", {$2}); }
 | SIZEOF LPAREN sizeof_type_specifier RPAREN { $$ = make_ast_node("SizeofType", "", {$3}); };
sizeof_type_specifier:
   INT { $$ = make_ast_node("Type", "int"); }
 | FLOAT { $$ = make_ast_node("Type", "float"); }
 | DOUBLE { $$ = make_ast_node("Type", "double"); }
 | CHAR { $$ = make_ast_node("Type", "char"); }
 | BOOL { $$ = make_ast_node("Type", "bool"); }
 | STRUCT IDENTIFIER { $$ = make_ast_node("Type", "struct " + take_text($2)); }
 | CLASS IDENTIFIER { $$ = make_ast_node("Type", "class " + take_text($2)); };
new_declarator_opt:
   %empty { $$ = make_ast_node("Empty"); }
 | LBRACKET expression RBRACKET { $$ = $2; };
new_initializer_opt:
   %empty { $$ = make_ast_list(); }
 | LPAREN argument_list_opt RPAREN { $$ = $2; };
delete_array_opt:
   %empty { $$ = make_ast_node("Empty"); }
 | LBRACKET RBRACKET { $$ = make_ast_node("ArrayDelete"); };
postfix_expression:
   primary_expression { $$ = $1; }
 | postfix_expression INC { $$ = make_ast_node("Postfix", "++", {$1}); }
 | postfix_expression DEC { $$ = make_ast_node("Postfix", "--", {$1}); }
 | postfix_expression LPAREN argument_list_opt RPAREN { $$ = make_ast_node("Call", "", {$1, $3}); }
 | postfix_expression LBRACKET expression RBRACKET { $$ = make_ast_node("ArrayAccess", "", {$1, $3}); }
 | postfix_expression DOT IDENTIFIER { $$ = make_ast_node("MemberAccess", "." + take_text($3), {$1}); }
 | postfix_expression ARROW IDENTIFIER { $$ = make_ast_node("MemberAccess", "->" + take_text($3), {$1}); }
 | postfix_expression SCOPE IDENTIFIER { $$ = make_ast_node("MemberAccess", "::" + take_text($3), {$1}); };
argument_list_opt:
   %empty { $$ = make_ast_list(); }
 | argument_list { $$ = $1; };
argument_list:
   assignment_expression { $$ = append_ast_list(make_ast_list(), $1); }
 | argument_list COMMA assignment_expression { $$ = append_ast_list($1, $3); };
primary_expression:
   IDENTIFIER { $$ = make_ast_node("Identifier", take_text($1)); }
 | INTEGER_LITERAL { $$ = make_ast_node("IntegerLiteral", take_text($1)); }
 | FLOAT_LITERAL { $$ = make_ast_node("FloatLiteral", take_text($1)); }
 | CHAR_LITERAL { $$ = make_ast_node("CharLiteral", take_text($1)); }
 | string_literal_seq { $$ = $1; }
 | TRUE { $$ = make_ast_node("BooleanLiteral", "true"); }
 | FALSE { $$ = make_ast_node("BooleanLiteral", "false"); }
 | THIS { $$ = make_ast_node("This"); }
 | INVALID { $$ = make_ast_node("Invalid"); }
 | LPAREN expression RPAREN { $$ = $2; };
string_literal_seq:
   STRING_LITERAL { $$ = make_ast_node("StringLiteral", take_text($1)); }
 | string_literal_seq STRING_LITERAL { $1->value += take_text($2); $$ = $1; };
%%

static int syntax_error_count = 0;

int parser_error_count()
{
   return syntax_error_count;
}

void yyerror(const char *message)
{
   syntax_error_count++;
   std::fprintf(stderr, "Syntax error at line %d, column %d: %s\n",
                yylloc.first_line, yylloc.first_column, message);
}
