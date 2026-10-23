%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>

#include "parser.tab.hpp"
#include "tokens.h"

using namespace std;

static string g_litbuf;
static int g_lit_start_line = 1;
static int g_char_units = 0;
static int g_lex_errors = 0;
static int g_column = 1;
static int g_comment_start_line = 1;
static YYLTYPE g_lit_loc;

static const map<string, int> g_parser_keywords = {
    {"bool", BOOL},       {"break", BREAK}, {"char", CHAR},
    {"case", CASE},      {"class", CLASS}, {"continue", CONTINUE},
    {"default", DEFAULT}, {"delete", DELETE}, {"do", DO},
    {"double", DOUBLE},  {"else", ELSE},   {"false", FALSE},
    {"float", FLOAT},    {"for", FOR},     {"if", IF},
    {"goto", GOTO},      {"int", INT},     {"new", NEW},
    {"enum", ENUM},      {"private", PRIVATE},
    {"protected", PROTECTED}, {"public", PUBLIC}, {"return", RETURN},
    {"sizeof", SIZEOF},  {"static", STATIC}, {"this", THIS},
    {"struct", STRUCT},  {"switch", SWITCH}, {"true", TRUE},
    {"typedef", TYPEDEF}, {"unsigned", UNSIGNED},
    {"void", VOID},      {"while", WHILE}
};

static const set<string> g_cpp_keywords = {
    "auto", "bool", "break", "case", "catch", "char", "class", "const", "continue",
    "default", "delete", "do", "double", "else", "enum", "extern", "false", "float",
    "for", "friend", "goto", "if", "inline", "int", "long", "namespace", "new", "nullptr",
    "operator", "private", "protected", "public", "return", "short", "signed", "sizeof",
    "static", "struct", "switch", "template", "this", "throw", "true", "try", "typedef",
    "typename", "union", "unsigned", "using", "virtual", "void", "volatile", "while"
};

static void add_token(const string &lexeme, const string &type)
{
    token_list_add(lexeme, type, yylineno);
}

static void set_semantic_text(const string &text)
{
    yylval.text = new string(text);
}

static void add_error_at(int line, const string &msg)
{
    ++g_lex_errors;
    cerr << "Lexical error at line " << line << ": " << msg << '\n';
}

static bool is_valid_escape(char c)
{
    static const string ok = "ntrbfav0\\'\"?";
    return ok.find(c) != string::npos;
}

static void update_location()
{
    yylloc.first_line = yylineno;
    yylloc.first_column = g_column;
    yylloc.last_line = yylineno;
    yylloc.last_column = g_column + yyleng - 1;
    g_column += yyleng;
}

void lexer_reset()
{
    g_litbuf.clear();
    g_lit_start_line = 1;
    g_char_units = 0;
    g_lex_errors = 0;
    g_column = 1;
    yylineno = 1;
}

int lexer_error_count()
{
    return g_lex_errors;
}

static void restore_literal_start()
{
    yylloc.first_line = g_lit_loc.first_line;
    yylloc.first_column = g_lit_loc.first_column;
}

static string ltrim(const char *text)
{
    while (*text == ' ' || *text == '\t') ++text;
    return string(text);
}

#define YY_USER_ACTION update_location();
%}

%option noyywrap nounput noinput yylineno
%x COMMENT STRLIT CHARLIT

DIGIT     [0-9]
LETTER    [a-zA-Z_]
ID        {LETTER}({LETTER}|{DIGIT})*
HEXNUM    0[xX][0-9a-fA-F]+
OCTNUM    0[0-7]+
INTNUM    {DIGIT}+
FLOATNUM  ({DIGIT}+"."{DIGIT}*|"."{DIGIT}+)([eE][+-]?{DIGIT}+)?|{DIGIT}+[eE][+-]?{DIGIT}+
BADHEX     0[xX]({LETTER}|{DIGIT})*
BADOCT     0[0-7]*[89]{DIGIT}*
BADMULTIDOT  {DIGIT}+("."{DIGIT}+){2,}
BADIDFROMDIGIT {DIGIT}+{LETTER}({LETTER}|{DIGIT})*

%%

[ \t\r]+            { }
\n                  { g_column = 1; }

"//"[^\n]*          { }

"/*"                { g_comment_start_line = yylineno; BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { g_column = 1; }
<COMMENT>.          { }
<COMMENT><<EOF>>    {
                        add_error_at(g_comment_start_line, "Unterminated block comment (missing closing */)");
                        BEGIN(INITIAL);
                        return INVALID;
                    }

^[ \t]*"#"[ \t]*include[^\n]* {
                        add_token(ltrim(yytext), "preprocessor_directive");
                        return INCLUDE_DIRECTIVE;
                    }
^[ \t]*"#"[^\n]*   {
                        add_error_at(yylineno, string("Unsupported preprocessor directive '") + ltrim(yytext) + "'");
                        return INVALID;
                    }

\"                  { g_lit_loc = yylloc; BEGIN(STRLIT); g_litbuf = "\""; g_lit_start_line = yylineno; }
<STRLIT>\\.         {
                        g_litbuf += yytext;
                        if (!is_valid_escape(yytext[1]))
                            add_error_at(yylineno, string("Invalid escape sequence '") + yytext + "' in string literal");
                    }
<STRLIT>\"          {
                        g_litbuf += "\"";
                        add_token(g_litbuf, "string_literal");
                        BEGIN(INITIAL);
                        restore_literal_start();
                        set_semantic_text(g_litbuf);
                        return STRING_LITERAL;
                    }
<STRLIT>\n          {
                        add_error_at(g_lit_start_line, "Unterminated string literal");
                        BEGIN(INITIAL);
                        g_column = 1;
                        restore_literal_start();
                        return INVALID;
                    }
<STRLIT>.           { g_litbuf += yytext; }
<STRLIT><<EOF>>     {
                        add_error_at(g_lit_start_line, "Unterminated string literal (reached end of file)");
                        BEGIN(INITIAL);
                        restore_literal_start();
                        return INVALID;
                    }

'                   { g_lit_loc = yylloc; BEGIN(CHARLIT); g_litbuf = ""; g_char_units = 0; g_lit_start_line = yylineno; }
<CHARLIT>\\.        {
                        g_litbuf += yytext;
                        g_char_units++;
                        if (!is_valid_escape(yytext[1]))
                            add_error_at(yylineno, string("Invalid escape sequence '") + yytext + "' in character literal");
                    }
<CHARLIT>'          {
                        BEGIN(INITIAL);
                        restore_literal_start();
                        if (g_char_units == 0) {
                            add_error_at(g_lit_start_line, "Empty character constant ''");
                            return INVALID;
                        }
                        if (g_char_units == 1) {
                            add_token("'" + g_litbuf + "'", "char_constant");
                            set_semantic_text("'" + g_litbuf + "'");
                            return CHAR_LITERAL;
                        }
                        add_error_at(g_lit_start_line, "Invalid multi-character constant '" + g_litbuf + "'");
                        return INVALID;
                    }
<CHARLIT>\n         {
                        add_error_at(g_lit_start_line, "Unterminated character literal");
                        BEGIN(INITIAL);
                        g_column = 1;
                        restore_literal_start();
                        return INVALID;
                    }
<CHARLIT>.          { g_litbuf += yytext; g_char_units++; }
<CHARLIT><<EOF>>    {
                        add_error_at(g_lit_start_line, "Unterminated character literal (reached end of file)");
                        BEGIN(INITIAL);
                        restore_literal_start();
                        return INVALID;
                    }

{FLOATNUM}          { add_token(yytext, "float_constant"); set_semantic_text(yytext); return FLOAT_LITERAL; }
{HEXNUM}            { add_token(yytext, "integer_constant"); set_semantic_text(yytext); return INTEGER_LITERAL; }
{OCTNUM}            { add_token(yytext, "integer_constant"); set_semantic_text(yytext); return INTEGER_LITERAL; }
{BADHEX}            { add_error_at(yylineno, string("Invalid hexadecimal constant '") + yytext + "'"); return INVALID; }
{BADOCT}            { add_error_at(yylineno, string("Invalid octal constant '") + yytext + "' (digits 8 and 9 are not allowed)"); return INVALID; }
{BADMULTIDOT}       { add_error_at(yylineno, string("Malformed number literal '") + yytext + "' (multiple decimal points)"); return INVALID; }
{BADIDFROMDIGIT}    { add_error_at(yylineno, string("Invalid token '") + yytext + "' (identifier cannot start with a digit)"); return INVALID; }
{INTNUM}            { add_token(yytext, "integer_constant"); set_semantic_text(yytext); return INTEGER_LITERAL; }

^[ \t]*"public"[ \t]*":"    { add_token(yytext, "keyword"); yyless(yyleng - 1); --g_column; return PUBLIC; }
^[ \t]*"private"[ \t]*":"   { add_token(yytext, "keyword"); yyless(yyleng - 1); --g_column; return PRIVATE; }
^[ \t]*"protected"[ \t]*":" { add_token(yytext, "keyword"); yyless(yyleng - 1); --g_column; return PROTECTED; }
^[ \t]*"default"[ \t]*":"   { add_token(yytext, "keyword"); yyless(yyleng - 1); --g_column; return DEFAULT; }
^[ \t]*{ID}[ \t]*":" {
                        add_token(yytext, "label");
                        string label(yytext);
                        label = ltrim(label.c_str());
                        set_semantic_text(label.substr(0, label.find(':')));
                        return LABEL;
                    }

{ID}                {
                        map<string, int>::const_iterator it = g_parser_keywords.find(yytext);
                        if (it != g_parser_keywords.end()) {
                            add_token(yytext, "keyword");
                            return it->second;
                        }
                        if (g_cpp_keywords.count(yytext))
                            add_token(yytext, "keyword");
                        else
                            add_token(yytext, "identifier");
                        set_semantic_text(yytext);
                        return IDENTIFIER;
                    }

"<<="               { add_token(yytext, "assignment_operator"); return SHL_ASSIGN; }
">>="               { add_token(yytext, "assignment_operator"); return SHR_ASSIGN; }
"&="                { add_token(yytext, "assignment_operator"); return BIT_AND_ASSIGN; }
"|="                { add_token(yytext, "assignment_operator"); return BIT_OR_ASSIGN; }
"^="                { add_token(yytext, "assignment_operator"); return BIT_XOR_ASSIGN; }
"+="                { add_token(yytext, "assignment_operator"); return PLUS_ASSIGN; }
"-="                { add_token(yytext, "assignment_operator"); return MINUS_ASSIGN; }
"*="                { add_token(yytext, "assignment_operator"); return STAR_ASSIGN; }
"/="                { add_token(yytext, "assignment_operator"); return SLASH_ASSIGN; }
"%="                { add_token(yytext, "assignment_operator"); return PERCENT_ASSIGN; }
"="                 { add_token(yytext, "assignment_operator"); return ASSIGN; }

"=="                { add_token(yytext, "relational_operator"); return EQ; }
"!="                { add_token(yytext, "relational_operator"); return NE; }
"<="                { add_token(yytext, "relational_operator"); return LE; }
">="                { add_token(yytext, "relational_operator"); return GE; }
"<"                 { add_token(yytext, "relational_operator"); return LT; }
">"                 { add_token(yytext, "relational_operator"); return GT; }

"&&"                { add_token(yytext, "logical_operator"); return AND; }
"||"                { add_token(yytext, "logical_operator"); return OR; }
"!"                 { add_token(yytext, "logical_operator"); return NOT; }

"<<"                { add_token(yytext, "bitwise_operator"); return SHL; }
">>"                { add_token(yytext, "bitwise_operator"); return SHR; }
"&"                 { add_token(yytext, "bitwise_operator"); return BIT_AND; }
"|"                 { add_token(yytext, "bitwise_operator"); return BIT_OR; }
"^"                 { add_token(yytext, "bitwise_operator"); return BIT_XOR; }
"~"                 { add_token(yytext, "bitwise_operator"); return BIT_NOT; }

"++"                { add_token(yytext, "increment_decrement_operator"); return INC; }
"--"                { add_token(yytext, "increment_decrement_operator"); return DEC; }

"+"                 { add_token(yytext, "arithmetic_operator"); return PLUS; }
"-"                 { add_token(yytext, "arithmetic_operator"); return MINUS; }
"*"                 { add_token(yytext, "arithmetic_operator"); return STAR; }
"/"                 { add_token(yytext, "arithmetic_operator"); return SLASH; }
"%"                 { add_token(yytext, "arithmetic_operator"); return PERCENT; }

"->"                { add_token(yytext, "member_operator"); return ARROW; }
"::"                { add_token(yytext, "member_operator"); return SCOPE; }
"."                 { add_token(yytext, "member_operator"); return DOT; }

"?"                 { add_token(yytext, "ternary_operator"); return QUESTION; }

"("                 { add_token(yytext, "punctuation"); return LPAREN; }
")"                 { add_token(yytext, "punctuation"); return RPAREN; }
"{"                 { add_token(yytext, "punctuation"); return LBRACE; }
"}"                 { add_token(yytext, "punctuation"); return RBRACE; }
"["                 { add_token(yytext, "punctuation"); return LBRACKET; }
"]"                 { add_token(yytext, "punctuation"); return RBRACKET; }
";"                 { add_token(yytext, "punctuation"); return SEMICOLON; }
","                 { add_token(yytext, "punctuation"); return COMMA; }
":"                 { add_token(yytext, "punctuation"); return COLON; }

.                   { add_error_at(yylineno, string("Illegal character '") + yytext + "'"); return INVALID; }

%%
