#ifndef AST_HPP
#define AST_HPP

#include <initializer_list>
#include <string>
#include <vector>

// A deliberately small, generic AST node.  The parser will use `kind` to
// describe the language construct, `value` for names/operators/literals, and
// `children` for its operands or nested statements.
struct AstNode {
    std::string kind;
    std::string value;
    int line;
    int column;
    std::vector<AstNode *> children;

    AstNode(std::string nodeKind, std::string nodeValue = "", int nodeLine = 0,
            int nodeColumn = 0);
    ~AstNode();

    AstNode(const AstNode &) = delete;
    AstNode &operator=(const AstNode &) = delete;

    void addChild(AstNode *child);
};

AstNode *make_ast_node(const std::string &kind, const std::string &value = "",
                       std::initializer_list<AstNode *> children = {}, int line = 0,
                       int column = 0);
AstNode *make_ast_list();
AstNode *append_ast_list(AstNode *list, AstNode *item);

void ast_set_root(AstNode *root);
AstNode *ast_root();
void ast_reset();
std::string ast_to_string(const AstNode *node, int indent = 0);

#endif
