#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>

#include "ast.hpp"
#include "ir.hpp"
#include "semantic.hpp"
#include "tokens.h"

extern FILE *yyin;

int yyparse();
void lexer_reset();
int lexer_error_count();
int parser_error_count();

int main(int argc, char **argv)
{
    bool print_ast = false;
    const char *ir_path = nullptr;
    const char *source_path = nullptr;

    if (argc == 2) {
        source_path = argv[1];
    } else if (argc == 3 && std::string(argv[1]) == "--ast") {
        print_ast = true;
        source_path = argv[2];
    } else if (argc == 4 && std::string(argv[1]) == "--ir") {
        ir_path = argv[2];
        source_path = argv[3];
    } else {
        std::cerr << "Usage: " << argv[0] << " [--ast] SOURCE_FILE\n"
                  << "       " << argv[0] << " --ir OUTPUT_FILE SOURCE_FILE\n";
        return 2;
    }

    FILE *input = std::fopen(source_path, "r");
    if (!input) {
        std::perror(source_path);
        return 2;
    }

    yyin = input;
    ast_reset();
    token_list_reset();
    lexer_reset();

    const int parse_result = yyparse();

    std::fclose(input);

    if (parse_result != 0 || parser_error_count() != 0 || lexer_error_count() != 0 ||
        !semantic_validate(ast_root())) {
        ast_reset();
        token_list_reset();
        return 1;
    }

    if (ir_path) {
        std::ofstream ir_file(ir_path);
        if (!ir_file) {
            std::perror(ir_path);
            ast_reset();
            token_list_reset();
            return 2;
        }
        ir_file << generate_ir(ast_root());
        std::cout << "IR written to " << ir_path << '\n';
    } else if (print_ast) {
        std::cout << ast_to_string(ast_root());
    } else {
        token_list_print();
    }
    ast_reset();
    token_list_reset();
    return 0;
}
