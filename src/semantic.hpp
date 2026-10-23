#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

#include "ast.hpp"

// Performs semantic checks that require information unavailable to the
// context-free grammar. It returns false after writing diagnostics to stderr.
bool semantic_validate(const AstNode *root);

#endif
