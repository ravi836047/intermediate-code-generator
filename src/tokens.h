#ifndef TOKENS_H
#define TOKENS_H

#include <string>

void token_list_reset();
void token_list_add(const std::string &lexeme, const std::string &type, int line);
void token_list_print();

#endif
