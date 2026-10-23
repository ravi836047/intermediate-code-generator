#include "ast.hpp"

#include <sstream>
#include <utility>

namespace {
AstNode *g_root = nullptr;

void write_node(const AstNode *node, std::ostringstream &out, int indent)
{
    if (!node) {
        return;
    }

    out << std::string(static_cast<std::size_t>(indent), ' ') << node->kind;
    if (!node->value.empty()) {
        out << "(" << node->value << ")";
    }
    out << '\n';

    for (const AstNode *child : node->children) {
        write_node(child, out, indent + 2);
    }
}
} // namespace

AstNode::AstNode(std::string nodeKind, std::string nodeValue, int nodeLine, int nodeColumn)
    : kind(std::move(nodeKind)), value(std::move(nodeValue)), line(nodeLine), column(nodeColumn)
{
}

AstNode::~AstNode()
{
    for (AstNode *child : children) {
        delete child;
    }
}

void AstNode::addChild(AstNode *child)
{
    if (child) {
        children.push_back(child);
    }
}

AstNode *make_ast_node(const std::string &kind, const std::string &value,
                       std::initializer_list<AstNode *> children, int line, int column)
{
    AstNode *node = new AstNode(kind, value, line, column);
    for (AstNode *child : children) {
        node->addChild(child);
    }
    return node;
}

AstNode *make_ast_list()
{
    return make_ast_node("List");
}

AstNode *append_ast_list(AstNode *list, AstNode *item)
{
    if (!list) {
        list = make_ast_list();
    }
    list->addChild(item);
    return list;
}

void ast_set_root(AstNode *root)
{
    if (g_root != root) {
        delete g_root;
        g_root = root;
    }
}

AstNode *ast_root()
{
    return g_root;
}

void ast_reset()
{
    delete g_root;
    g_root = nullptr;
}

std::string ast_to_string(const AstNode *node, int indent)
{
    std::ostringstream out;
    write_node(node, out, indent);
    return out.str();
}
