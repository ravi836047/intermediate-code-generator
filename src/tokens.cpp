#include "tokens.h"

#include <iomanip>
#include <iostream>
#include <vector>

struct TokenRow {
    std::string lexeme;
    std::string type;
    int line;
};

static std::vector<TokenRow> rows;

void token_list_reset()
{
    rows.clear();
}

void token_list_add(const std::string &lexeme, const std::string &type, int line)
{
    rows.push_back({lexeme, type, line});
}

void token_list_print()
{
    std::cout << std::left << std::setw(28) << "Token"
              << "Token_Type\n";
    std::cout << std::string(52, '-') << '\n';

    for (const TokenRow &row : rows) {
        std::cout << std::left << std::setw(28) << row.lexeme
                  << row.type << '\n';
    }

    std::cout << std::string(52, '-') << '\n';
    std::cout << "Total tokens: " << rows.size() << '\n';
}
