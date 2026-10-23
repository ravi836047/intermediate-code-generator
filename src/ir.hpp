#ifndef IR_HPP
#define IR_HPP

#include <string>

#include "ast.hpp"

// Produces a readable three-address-code (TAC) representation from the AST.
// The generator deliberately consumes the existing generic AST; it does not
// alter parsing or semantic-analysis behaviour.
std::string generate_ir(const AstNode *root);

#endif
