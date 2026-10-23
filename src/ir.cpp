#include "ir.hpp"

#include <sstream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace {
class IrGenerator {
public:
    std::string run(const AstNode *root)
    {
        collectSymbols(root);
        emit("# Three-address intermediate representation");
        generateNode(root);
        return out_.str();
    }

private:
    std::ostringstream out_;
    int temporary_ = 0;
    int label_ = 0;
    bool global_ = true;
    std::vector<std::string> breaks_;
    std::vector<std::string> continues_;
    std::map<std::string, std::string> variables_;
    std::map<std::string, std::string> aliases_;
    std::map<std::string, std::vector<std::pair<std::vector<std::string>, std::string>>> functions_;
    std::set<std::string> aggregates_;

    void emit(const std::string &line) { out_ << line << '\n'; }
    std::string temp() { return "t" + std::to_string(++temporary_); }
    std::string label(const std::string &prefix) { return prefix + "_" + std::to_string(++label_); }
    static const AstNode *child(const AstNode *node, std::size_t index)
    {
        return node && index < node->children.size() ? node->children[index] : nullptr;
    }
    static bool isEmpty(const AstNode *node) { return !node || node->kind == "Empty"; }

    std::string resolveAlias(const std::string &name) const
    {
        const auto found = aliases_.find(name);
        return found == aliases_.end() ? name : found->second;
    }

    std::string declaratorName(const AstNode *node) const
    {
        if (!node) return "<anonymous>";
        if (node->kind == "Identifier") return node->value;
        for (const AstNode *part : node->children) {
            const std::string name = declaratorName(part);
            if (name != "<anonymous>") return name;
        }
        return "<anonymous>";
    }
    std::string typeName(const AstNode *node) const
    {
        if (!node) return "auto";
        if (node->kind == "Type") return node->value;
        if ((node->kind == "StaticType" || node->kind == "TypedefType") && child(node, 0))
            return typeName(child(node, 0));
        return node->value.empty() ? node->kind : node->value;
    }
    std::string declaratorSuffix(const AstNode *node) const
    {
        if (!node) return "";
        if (node->kind == "Pointer") return node->value;
        if (node->kind == "Reference") return node->value;
        if (node->kind == "ArrayDeclarator") return "[]";
        std::string result;
        for (const AstNode *part : node->children) result += declaratorSuffix(part);
        return result;
    }
    std::string declaredType(const AstNode *specification, const AstNode *declarator) const
    {
        return typeName(specification) + declaratorSuffix(declarator);
    }
    std::vector<std::string> parameterTypes(const AstNode *parameters) const
    {
        std::vector<std::string> result;
        if (!parameters) return result;
        for (const AstNode *parameter : parameters->children) {
            if (parameter->kind != "Parameter") continue;
            const std::string type = declaredType(child(parameter, 0), child(parameter, 1));
            if (!child(parameter, 1) && type == "void") continue;
            result.push_back(type);
        }
        return result;
    }
    std::string symbol(const std::string &name, const std::vector<std::string> &parameters) const
    {
        std::string result = name + "$" + std::to_string(parameters.size());
        for (const std::string &parameter : parameters) result += "$" + parameter;
        return result;
    }
    void collectSymbols(const AstNode *node)
    {
        if (!node) return;
        if (node->kind == "ClassDefinition" || node->kind == "StructDefinition") aggregates_.insert(node->value);
        if (node->kind == "FunctionDefinition") {
            const std::vector<std::string> parameters = parameterTypes(child(node, 1));
            functions_[node->value].push_back({parameters, symbol(node->value, parameters)});
            return;
        }
        for (const AstNode *part : node->children) collectSymbols(part);
    }
    std::string expressionType(const AstNode *node) const
    {
        if (!node) return "";
        if (node->kind == "IntegerLiteral") return "int";
        if (node->kind == "FloatLiteral") return "float";
        if (node->kind == "CharLiteral") return "char";
        if (node->kind == "BooleanLiteral") return "bool";
        if (node->kind == "StringLiteral") return "char*";
        if (node->kind == "Identifier") { const auto found = variables_.find(resolveAlias(node->value)); return found == variables_.end() ? "" : found->second; }
        if (node->kind == "Unary" && node->value == "&") return expressionType(child(node, 0)) + "*";
        return "";
    }
    std::string callSymbol(const AstNode *callee, const AstNode *arguments) const
    {
        if (!callee || callee->kind != "Identifier") return "<indirect-call>";
        const auto candidates = functions_.find(callee->value);
        if (candidates == functions_.end()) return callee->value;
        std::vector<std::string> argumentTypes;
        if (arguments) for (const AstNode *argument : arguments->children) argumentTypes.push_back(expressionType(argument));
        for (const auto &candidate : candidates->second)
            if (candidate.first == argumentTypes) return candidate.second;
        for (const auto &candidate : candidates->second)
            if (candidate.first.size() == argumentTypes.size()) return candidate.second;
        return callee->value;
    }
    std::string lvalue(const AstNode *node)
    {
        if (!node) return "<invalid-lvalue>";
        if (node->kind == "Identifier") return resolveAlias(node->value);
        if (node->kind == "Unary" && node->value == "*") return "*" + expr(child(node, 0));
        if (node->kind == "ArrayAccess") return lvalue(child(node, 0)) + "[" + expr(child(node, 1)) + "]";
        if (node->kind == "MemberAccess") return lvalue(child(node, 0)) + node->value;
        return expr(node);
    }
    void condition(const AstNode *node, const std::string &trueLabel, const std::string &falseLabel)
    {
        if (node && node->kind == "Binary" && node->value == "&&") {
            const std::string next = label("and_rhs");
            condition(child(node, 0), next, falseLabel);
            emit(next + ":");
            condition(child(node, 1), trueLabel, falseLabel);
        } else if (node && node->kind == "Binary" && node->value == "||") {
            const std::string next = label("or_rhs");
            condition(child(node, 0), trueLabel, next);
            emit(next + ":");
            condition(child(node, 1), trueLabel, falseLabel);
        } else {
            emit("if " + expr(node) + " goto " + trueLabel);
            emit("goto " + falseLabel);
        }
    }
    std::string expr(const AstNode *node)
    {
        if (!node || node->kind == "Empty") return "";
        if (node->kind == "Identifier" || node->kind == "IntegerLiteral" || node->kind == "FloatLiteral" ||
            node->kind == "CharLiteral" || node->kind == "StringLiteral" || node->kind == "BooleanLiteral")
            return node->kind == "Identifier" ? resolveAlias(node->value) : node->value;
        if (node->kind == "This") return "this";
        if (node->kind == "Binary") {
            if (node->value == "&&" || node->value == "||") {
                const std::string result = temp(), yes = label("logic_true"), no = label("logic_false"), done = label("logic_end");
                condition(node, yes, no);
                emit(yes + ":"); emit(result + " = 1"); emit("goto " + done);
                emit(no + ":"); emit(result + " = 0"); emit(done + ":");
                return result;
            }
            const std::string result = temp();
            emit(result + " = " + expr(child(node, 0)) + " " + node->value + " " + expr(child(node, 1)));
            return result;
        }
        if (node->kind == "Unary") {
            const std::string result = temp();
            emit(result + " = " + node->value + expr(child(node, 0)));
            return result;
        }
        if (node->kind == "Assignment") {
            const std::string left = lvalue(child(node, 0));
            const std::string right = expr(child(node, 1));
            if (node->value == "=") emit(left + " = " + right);
            else emit(left + " = " + left + " " + node->value.substr(0, node->value.size() - 1) + " " + right);
            return left;
        }
        if (node->kind == "Comma") { expr(child(node, 0)); return expr(child(node, 1)); }
        if (node->kind == "Conditional") {
            const std::string result = temp(), yes = label("ternary_true"), no = label("ternary_false"), done = label("ternary_end");
            condition(child(node, 0), yes, no);
            emit(yes + ":"); emit(result + " = " + expr(child(node, 1))); emit("goto " + done);
            emit(no + ":"); emit(result + " = " + expr(child(node, 2))); emit(done + ":");
            return result;
        }
        if (node->kind == "Prefix" || node->kind == "Postfix") {
            const std::string place = lvalue(child(node, 0));
            const std::string old = node->kind == "Postfix" ? temp() : place;
            if (node->kind == "Postfix") emit(old + " = " + place);
            emit(place + " = " + place + (node->value == "++" ? " + 1" : " - 1"));
            return old;
        }
        if (node->kind == "ArrayAccess") {
            const std::string result = temp(); emit(result + " = " + lvalue(node)); return result;
        }
        if (node->kind == "MemberAccess") {
            const std::string result = temp(); emit(result + " = " + lvalue(node)); return result;
        }
        if (node->kind == "Call") {
            const AstNode *arguments = child(node, 1);
            std::string target = callSymbol(child(node, 0), arguments);
            std::size_t count = arguments ? arguments->children.size() : 0;
            if (child(node, 0) && child(node, 0)->kind == "MemberAccess") {
                const AstNode *member = child(node, 0);
                const std::string object = lvalue(child(member, 0));
                std::string classType = expressionType(child(member, 0));
                while (!classType.empty() && (classType.back() == '*' || classType.back() == '&')) classType.pop_back();
                std::string memberName = member->value;
                memberName = memberName.rfind("->", 0) == 0 ? memberName.substr(2) : memberName.substr(1);
                target = classType.empty() ? "<member-call>" : classType + "::" + memberName;
                emit("param " + (member->value.rfind("->", 0) == 0 ? object : "&" + object));
                ++count;
            }
            if (arguments) for (const AstNode *argument : arguments->children) emit("param " + expr(argument));
            const std::string result = temp();
            emit(result + " = call " + target + ", " + std::to_string(count));
            return result;
        }
        if (node->kind == "New") {
            const std::string result = temp();
            const AstNode *size = child(node, 1), *arguments = child(node, 2);
            if (!isEmpty(size)) emit(result + " = new " + typeName(child(node, 0)) + "[" + expr(size) + "]");
            else { if (arguments) for (const AstNode *argument : arguments->children) emit("param " + expr(argument)); emit(result + " = new " + typeName(child(node, 0))); }
            return result;
        }
        if (node->kind == "Delete") { emit(std::string("delete") + (child(node, 0) && child(node, 0)->kind == "ArrayDelete" ? "[] " : " ") + expr(child(node, 1))); return ""; }
        if (node->kind == "Sizeof" || node->kind == "SizeofType") { const std::string result = temp(); emit(result + " = sizeof " + (node->kind == "Sizeof" ? expr(child(node, 0)) : typeName(child(node, 0)))); return result; }
        return "<" + node->kind + ">";
    }
    void lowerInitializer(const std::string &name, const AstNode *initial, std::vector<std::size_t> indices)
    {
        if (initial && initial->kind == "InitializerList") {
            const AstNode *values = child(initial, 0);
            if (values) for (std::size_t i = 0; i < values->children.size(); ++i) {
                std::vector<std::size_t> nested = indices;
                nested.push_back(i);
                lowerInitializer(name, values->children[i], std::move(nested));
            }
            return;
        }
        std::string target = name;
        for (const std::size_t index : indices) target += "[" + std::to_string(index) + "]";
        emit(target + " = " + expr(initial));
    }
    void declaration(const AstNode *node)
    {
        const AstNode *specification = child(node, 0);
        const bool isStatic = specification && specification->kind == "StaticType";
        const bool isTypedef = specification && specification->kind == "TypedefType";
        const AstNode *items = child(node, 1);
        if (!items) return;
        for (const AstNode *item : items->children) {
            const AstNode *decl = child(item, 0);
            const std::string name = declaratorName(decl);
            const std::string type = declaredType(specification, decl);
            variables_[name] = type;
            if (isTypedef) { emit("type " + name + " = " + typeName(specification)); continue; }
            const bool reference = type.find('&') != std::string::npos;
            emit(std::string(global_ ? (isStatic ? "static global " : "global ")
                                     : (isStatic ? "static var " : "var ")) + name + " : " + type);
            if (item->children.size() > 1) {
                const AstNode *initial = child(item, 1);
                if (item->kind == "DirectInitializer") {
                    const std::string baseType = typeName(specification);
                    if (aggregates_.count(baseType)) {
                        emit("param &" + name);
                        if (initial) for (const AstNode *argument : initial->children) emit("param " + expr(argument));
                        emit("call " + baseType + "::" + baseType + ", " + std::to_string((initial ? initial->children.size() : 0) + 1) + " # construct " + name);
                    } else if (initial && !initial->children.empty()) {
                        emit(name + " = " + expr(initial->children.front()));
                    }
                } else if (reference) {
                    const std::string target = lvalue(initial);
                    aliases_[name] = target;
                    emit("alias " + name + " -> " + target);
                } else if (initial && initial->kind == "InitializerList") lowerInitializer(name, initial, {});
                else emit(name + " = " + expr(initial));
            }
        }
    }
    void function(const AstNode *node, const std::string &qualifiedName = "")
    {
        std::string name = qualifiedName;
        if (name.empty()) {
            const std::vector<std::string> parameters = parameterTypes(child(node, 1));
            name = symbol(node->value, parameters);
        }
        emit("func " + name + ":");
        if (!qualifiedName.empty()) {
            const std::size_t separator = qualifiedName.find("::");
            emit("param this : " + qualifiedName.substr(0, separator) + "*");
        }
        const AstNode *parameters = child(node, 1);
        if (parameters) for (const AstNode *parameter : parameters->children) {
            // `void` as the sole parameter is the C/C++ spelling of an empty
            // parameter list, not an unnamed runtime parameter.
            if (!child(parameter, 1) && typeName(child(parameter, 0)) == "void") continue;
            emit("param " + declaratorName(child(parameter, 1)) + " : " + typeName(child(parameter, 0)));
        }
        const bool oldGlobal = global_; global_ = false;
        generateNode(child(node, 2));
        global_ = oldGlobal;
        emit("endfunc " + name);
    }
    void constructor(const AstNode *node, const std::string &className)
    {
        const std::string name = className + "::" + className;
        emit("func " + name + ":");
        emit("param this : " + className + "*");
        const AstNode *parameters = child(node, 0);
        if (parameters) for (const AstNode *parameter : parameters->children)
            emit("param " + declaratorName(child(parameter, 1)) + " : " + declaredType(child(parameter, 0), child(parameter, 1)));
        const bool oldGlobal = global_; global_ = false;
        generateNode(child(node, 2));
        global_ = oldGlobal;
        emit("return");
        emit("endfunc " + name);
    }
    void generateNode(const AstNode *node)
    {
        if (!node) return;
        if (node->kind == "Program" || node->kind == "List") { for (const AstNode *part : node->children) generateNode(part); return; }
        if (node->kind == "FunctionDefinition") { function(node); return; }
        if (node->kind == "Declaration") { declaration(node); return; }
        if (node->kind == "Block") { generateNode(child(node, 0)); return; }
        if (node->kind == "ExpressionStatement") { expr(child(node, 0)); return; }
        if (node->kind == "Return") { const std::string value = expr(child(node, 0)); emit(value.empty() ? "return" : "return " + value); return; }
        if (node->kind == "If") { const std::string yes = label("if_true"), no = label("if_false"), done = label("if_end"); condition(child(node, 0), yes, no); emit(yes + ":"); generateNode(child(node, 1)); emit("goto " + done); emit(no + ":"); if (node->children.size() > 2) generateNode(child(node, 2)); emit(done + ":"); return; }
        if (node->kind == "While") { const std::string test = label("while_test"), body = label("while_body"), done = label("while_end"); emit(test + ":"); condition(child(node, 0), body, done); emit(body + ":"); breaks_.push_back(done); continues_.push_back(test); generateNode(child(node, 1)); continues_.pop_back(); breaks_.pop_back(); emit("goto " + test); emit(done + ":"); return; }
        if (node->kind == "DoWhile") { const std::string body = label("do_body"), test = label("do_test"), done = label("do_end"); emit(body + ":"); breaks_.push_back(done); continues_.push_back(test); generateNode(child(node, 0)); continues_.pop_back(); breaks_.pop_back(); emit(test + ":"); condition(child(node, 1), body, done); emit(done + ":"); return; }
        if (node->kind == "For") { const std::string test = label("for_test"), body = label("for_body"), step = label("for_step"), done = label("for_end"); const AstNode *initial = child(node, 0); if (initial && initial->kind == "Declaration") generateNode(initial); else expr(initial); emit(test + ":"); if (isEmpty(child(node, 1))) emit("goto " + body); else condition(child(node, 1), body, done); emit(body + ":"); breaks_.push_back(done); continues_.push_back(step); generateNode(child(node, 3)); continues_.pop_back(); breaks_.pop_back(); emit(step + ":"); expr(child(node, 2)); emit("goto " + test); emit(done + ":"); return; }
        if (node->kind == "Switch") { generateSwitch(node); return; }
        if (node->kind == "Break") { if (!breaks_.empty()) emit("goto " + breaks_.back()); return; }
        if (node->kind == "Continue") { if (!continues_.empty()) emit("goto " + continues_.back()); return; }
        if (node->kind == "Goto") { emit("goto " + node->value); return; }
        if (node->kind == "Label") { emit(node->value + ":"); generateNode(child(node, 0)); return; }
        if (node->kind == "EnumDefinition") {
            emit("# enum " + declaratorName(child(node, 0)));
            int next = 0;
            const AstNode *enumerators = child(node, 1);
            if (enumerators) for (const AstNode *entry : enumerators->children) {
                if (entry->children.size() == 1 && child(entry, 0)->kind == "IntegerLiteral") next = std::stoi(child(entry, 0)->value);
                emit("const " + entry->value + " = " + std::to_string(next++));
                variables_[entry->value] = "int";
            }
            return;
        }
        if (node->kind == "StructDefinition" || node->kind == "ClassDefinition") {
            std::string header = "type " + node->value;
            if (node->kind == "ClassDefinition") {
                const AstNode *bases = child(node, 0);
                if (bases && !bases->children.empty()) {
                    header += " extends ";
                    for (std::size_t i = 0; i < bases->children.size(); ++i) {
                        if (i) header += ", ";
                        header += bases->children[i]->value;
                    }
                }
            }
            emit(header);
            // Data members define layout only. Member functions do have
            // executable bodies, so retain them in the IR with qualified names.
            const AstNode *members = node->kind == "StructDefinition" ? child(node, 0) : child(node, 1);
            if (members) for (const AstNode *member : members->children) {
                if (member && member->kind == "Declaration") {
                    const AstNode *items = child(member, 1);
                    if (items) for (const AstNode *item : items->children)
                        emit("field " + declaratorName(child(item, 0)) + " : " + declaredType(child(member, 0), child(item, 0)));
                }
                if (member && member->kind == "FunctionDefinition") function(member, node->value + "::" + member->value);
                else if (member && member->kind == "ConstructorDefinition") constructor(member, node->value);
            }
            return;
        }
    }
    void generateSwitch(const AstNode *node)
    {
        const std::string value = expr(child(node, 0)), done = label("switch_end");
        const AstNode *sections = child(node, 1); std::vector<std::string> labels; std::string defaultLabel = done;
        if (sections) for (const AstNode *section : sections->children) {
            labels.push_back(label("case"));
            if (child(section, 0) && child(section, 0)->kind == "Default") defaultLabel = labels.back();
        }
        if (sections) for (std::size_t i = 0; i < sections->children.size(); ++i) {
            const AstNode *tag = child(sections->children[i], 0);
            if (tag && tag->kind == "Case") emit("if " + value + " == " + expr(child(tag, 0)) + " goto " + labels[i]);
        }
        emit("goto " + defaultLabel); breaks_.push_back(done);
        if (sections) for (std::size_t i = 0; i < sections->children.size(); ++i) { emit(labels[i] + ":"); generateNode(child(sections->children[i], 1)); }
        breaks_.pop_back(); emit(done + ":");
    }
};
} // namespace

std::string generate_ir(const AstNode *root)
{
    return IrGenerator().run(root);
}
