#include "semantic.hpp"

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {
struct TypeInfo {
    std::string base;
    int pointer_depth = 0;
    bool is_reference = false;
    bool is_array = false;
    int array_dimensions = 0;
};

struct MemberInfo {
    TypeInfo type;
    std::string access;
};

struct AggregateInfo {
    bool is_class = false;
    std::vector<std::string> bases;
    std::map<std::string, MemberInfo> members;
};

struct FunctionInfo {
    std::string name;
    TypeInfo return_type;
    std::vector<TypeInfo> parameters;
};

using AggregateTable = std::map<std::string, AggregateInfo>;
using FunctionTable = std::map<std::string, std::vector<FunctionInfo>>;
using VariableTable = std::map<std::string, TypeInfo>;

int g_errors = 0;

void error(const std::string &message)
{
    ++g_errors;
    std::cerr << "Semantic error: " << message << '\n';
}

std::string remove_tag(const std::string &name)
{
    if (name.rfind("struct ", 0) == 0) {
        return name.substr(7);
    }
    if (name.rfind("class ", 0) == 0) {
        return name.substr(6);
    }
    if (name.rfind("enum ", 0) == 0) {
        return name.substr(5);
    }
    return name;
}

const AstNode *underlying_type_node(const AstNode *node)
{
    if (!node) {
        return nullptr;
    }
    if ((node->kind == "StaticType" || node->kind == "TypedefType") &&
        !node->children.empty()) {
        return underlying_type_node(node->children.front());
    }
    return node;
}

TypeInfo type_from_node(const AstNode *node)
{
    TypeInfo type;
    const AstNode *underlying = underlying_type_node(node);
    if (underlying && underlying->kind == "Type") {
        type.base = remove_tag(underlying->value);
    }
    return type;
}

const AstNode *find_node(const AstNode *node, const std::string &kind)
{
    if (!node) {
        return nullptr;
    }
    if (node->kind == kind) {
        return node;
    }
    for (const AstNode *child : node->children) {
        if (const AstNode *found = find_node(child, kind)) {
            return found;
        }
    }
    return nullptr;
}

int count_nodes(const AstNode *node, const std::string &kind)
{
    if (!node) {
        return 0;
    }
    int count = node->kind == kind ? 1 : 0;
    for (const AstNode *child : node->children) {
        count += count_nodes(child, kind);
    }
    return count;
}

std::string declarator_name(const AstNode *node)
{
    const AstNode *identifier = find_node(node, "Identifier");
    return identifier ? identifier->value : "";
}

TypeInfo type_from_declarator(const AstNode *type_node, const AstNode *declarator)
{
    TypeInfo type = type_from_node(type_node);
    if (!declarator) {
        return type;
    }

    if (const AstNode *pointer = find_node(declarator, "Pointer")) {
        type.pointer_depth = static_cast<int>(pointer->value.size());
    }
    if (const AstNode *reference = find_node(declarator, "Reference")) {
        type.is_reference = reference->value == "&";
    }
    type.array_dimensions = count_nodes(declarator, "ArrayDeclarator");
    type.is_array = type.array_dimensions > 0;
    return type;
}

const AstNode *declarator_of(const AstNode *initializer)
{
    return initializer && !initializer->children.empty()
        ? initializer->children.front()
        : nullptr;
}

std::string expression_key(const AstNode *node)
{
    if (!node) {
        return "";
    }
    std::string key = node->kind + "(" + node->value + ")";
    for (const AstNode *child : node->children) {
        key += "[" + expression_key(child) + "]";
    }
    return key;
}

const AstNode *body_of(const AstNode *function)
{
    if (!function || function->children.size() < 3) {
        return nullptr;
    }
    return function->children[2];
}

const AstNode *parameters_of(const AstNode *function)
{
    if (!function || function->children.empty()) {
        return nullptr;
    }
    return function->kind == "FunctionDefinition" ? function->children[1]
                                                    : function->children[0];
}

TypeInfo return_type_of(const AstNode *function)
{
    return function && function->kind == "FunctionDefinition" && !function->children.empty()
        ? type_from_node(function->children[0])
        : TypeInfo{};
}

FunctionInfo function_info(const AstNode *function)
{
    FunctionInfo info;
    info.name = function ? function->value : "";
    info.return_type = return_type_of(function);
    const AstNode *parameters = parameters_of(function);
    if (!parameters) {
        return info;
    }

    for (const AstNode *parameter : parameters->children) {
        if (parameter->kind != "Parameter" || parameter->children.empty()) {
            continue;
        }
        const AstNode *declarator = parameter->children.size() > 1
            ? parameter->children[1]
            : nullptr;
        info.parameters.push_back(type_from_declarator(parameter->children[0], declarator));
    }
    return info;
}

void collect_aggregate(const AstNode *node, AggregateTable &aggregates)
{
    if (!node || (node->kind != "ClassDefinition" && node->kind != "StructDefinition")) {
        return;
    }

    AggregateInfo info;
    info.is_class = node->kind == "ClassDefinition";
    std::string access = info.is_class ? "private" : "public";
    const AstNode *members = nullptr;

    if (info.is_class) {
        if (node->children.size() > 0) {
            for (const AstNode *base : node->children[0]->children) {
                if (base->kind == "BaseClass") {
                    info.bases.push_back(base->value);
                }
            }
        }
        if (node->children.size() > 1) {
            members = node->children[1];
        }
    } else if (!node->children.empty()) {
        members = node->children[0];
    }

    if (members) {
        for (const AstNode *member : members->children) {
            if (member->kind == "AccessSpecifier") {
                access = member->value;
                continue;
            }
            // Methods participate in member lookup just like data members.  This
            // lets expressions such as object.method() pass the same existence
            // and access checks as object.field.
            if (member->kind == "FunctionDefinition") {
                info.members[member->value] = {return_type_of(member), access};
                continue;
            }
            if (member->kind != "Declaration" || member->children.size() < 2) {
                continue;
            }

            const AstNode *type_node = member->children[0];
            for (const AstNode *initializer : member->children[1]->children) {
                const AstNode *declarator = declarator_of(initializer);
                const std::string name = declarator_name(declarator);
                if (!name.empty()) {
                    info.members[name] = {type_from_declarator(type_node, declarator), access};
                }
            }
        }
    }
    aggregates[node->value] = info;
}

void collect_aggregates(const AstNode *node, AggregateTable &aggregates)
{
    if (!node) {
        return;
    }
    collect_aggregate(node, aggregates);
    for (const AstNode *child : node->children) {
        collect_aggregates(child, aggregates);
    }
}

void collect_functions(const AstNode *node, FunctionTable &functions)
{
    if (!node) {
        return;
    }
    if (node->kind == "FunctionDefinition") {
        functions[node->value].push_back(function_info(node));
        return;
    }
    if (node->kind == "ClassDefinition" || node->kind == "StructDefinition") {
        return;
    }
    for (const AstNode *child : node->children) {
        collect_functions(child, functions);
    }
}

void collect_typedefs(const AstNode *node, std::map<std::string, TypeInfo> &typedefs)
{
    if (!node) {
        return;
    }
    if (node->kind == "Declaration" && node->children.size() >= 2 &&
        node->children[0]->kind == "TypedefType") {
        for (const AstNode *initializer : node->children[1]->children) {
            const AstNode *declarator = declarator_of(initializer);
            const std::string name = declarator_name(declarator);
            const TypeInfo type = type_from_declarator(node->children[0], declarator);
            const auto existing = typedefs.find(name);
            if (existing != typedefs.end() && existing->second.base != type.base) {
                error("conflicting typedef declaration for '" + name + "'");
            } else if (!name.empty()) {
                typedefs[name] = type;
            }
        }
    }
    for (const AstNode *child : node->children) {
        collect_typedefs(child, typedefs);
    }
}

const MemberInfo *find_member(const AggregateTable &aggregates, const std::string &aggregate,
                              const std::string &member)
{
    const auto aggregate_it = aggregates.find(aggregate);
    if (aggregate_it == aggregates.end()) {
        return nullptr;
    }

    const auto direct = aggregate_it->second.members.find(member);
    if (direct != aggregate_it->second.members.end()) {
        return &direct->second;
    }
    for (const std::string &base : aggregate_it->second.bases) {
        if (const MemberInfo *inherited = find_member(aggregates, base, member)) {
            return inherited;
        }
    }
    return nullptr;
}

struct Context {
    std::string current_class;
    TypeInfo expected_return;
    int loop_depth = 0;
    int switch_depth = 0;
};

TypeInfo infer_type(const AstNode *node, const VariableTable &variables,
                    const AggregateTable &aggregates, const FunctionTable &functions,
                    const Context &context);

bool compatible(const TypeInfo &target, const TypeInfo &source)
{
    if (target.base.empty() || source.base.empty()) {
        return true;
    }
    if (target.pointer_depth != source.pointer_depth) {
        return false;
    }
    if (target.base == source.base) {
        return true;
    }
    const auto numeric = [](const std::string &type) {
        return type == "bool" || type == "char" || type == "int" ||
               type == "unsigned int" || type == "float" || type == "double";
    };
    return target.pointer_depth == 0 && source.pointer_depth == 0 &&
           numeric(target.base) && numeric(source.base);
}

// Returns the relative conversion cost used to choose an overload. A direct
// type match is better than a numeric conversion; tied best conversions are
// deliberately diagnosed as ambiguous.
int conversion_rank(const TypeInfo &target, const TypeInfo &source)
{
    if (!compatible(target, source)) {
        return 1000000;
    }
    return target.base == source.base && target.pointer_depth == source.pointer_depth ? 0 : 1;
}

void record_declaration(const AstNode *node, VariableTable &variables)
{
    if (!node || node->kind != "Declaration" || node->children.size() < 2) {
        return;
    }
    for (const AstNode *initializer : node->children[1]->children) {
        const AstNode *declarator = declarator_of(initializer);
        const std::string name = declarator_name(declarator);
        if (!name.empty()) {
            variables[name] = type_from_declarator(node->children[0], declarator);
        }
    }
}

void record_parameters(const AstNode *parameters, VariableTable &variables)
{
    if (!parameters) {
        return;
    }
    for (const AstNode *parameter : parameters->children) {
        if (parameter->kind != "Parameter" || parameter->children.empty()) {
            continue;
        }
        const AstNode *declarator = parameter->children.size() > 1 ? parameter->children[1] : nullptr;
        const std::string name = declarator_name(declarator);
        if (!name.empty()) {
            variables[name] = type_from_declarator(parameter->children[0], declarator);
        }
    }
}

void collect_labels(const AstNode *node, std::set<std::string> &labels,
                    std::set<std::string> &gotos)
{
    if (!node) {
        return;
    }
    if (node->kind == "Label") {
        if (!labels.insert(node->value).second) {
            error("duplicate label '" + node->value + "'");
        }
    } else if (node->kind == "Goto") {
        gotos.insert(node->value);
    }
    for (const AstNode *child : node->children) {
        collect_labels(child, labels, gotos);
    }
}

bool validate(const AstNode *node, VariableTable variables, const AggregateTable &aggregates,
              const FunctionTable &functions, Context context);

void validate_declaration(const AstNode *node, VariableTable &variables,
                          const AggregateTable &aggregates, const FunctionTable &functions,
                          const Context &context)
{
    if (!node || node->children.size() < 2) {
        return;
    }
    for (const AstNode *initializer : node->children[1]->children) {
        const AstNode *declarator = declarator_of(initializer);
        const TypeInfo declared = type_from_declarator(node->children[0], declarator);
        const std::string name = declarator_name(declarator);

        if (declared.is_reference && initializer->children.size() < 2) {
            error("reference '" + name + "' must be initialized");
        }
        if (const AstNode *array = find_node(declarator, "ArrayDeclarator")) {
            if (array->children.size() > 1 && array->children[1]->kind == "IntegerLiteral" &&
                array->children[1]->value == "0") {
                error("array '" + name + "' cannot have size zero");
            }
        }

        variables[name] = declared;
        if (initializer->children.size() > 1) {
            const AstNode *value = initializer->children[1];
            const TypeInfo source = infer_type(value, variables, aggregates, functions, context);
            if (declared.pointer_depth > 0 && source.pointer_depth == 0 && !source.base.empty()) {
                error("cannot initialize pointer '" + name + "' with non-pointer value");
            }
            validate(value, variables, aggregates, functions, context);
        }
    }
}

void validate_call(const AstNode *node, const VariableTable &variables,
                   const AggregateTable &aggregates, const FunctionTable &functions,
                   const Context &context)
{
    if (!node || node->children.size() < 2 || node->children[0]->kind != "Identifier") {
        return;
    }
    const std::string &name = node->children[0]->value;
    const auto overloads = functions.find(name);
    if (overloads == functions.end()) {
        return;
    }

    const AstNode *arguments = node->children[1];
    std::vector<TypeInfo> argument_types;
    for (const AstNode *argument : arguments->children) {
        argument_types.push_back(infer_type(argument, variables, aggregates, functions, context));
    }

    std::vector<const FunctionInfo *> arity_matches;
    int best_rank = 1000000;
    int best_count = 0;
    for (const FunctionInfo &candidate : overloads->second) {
        if (candidate.parameters.size() != argument_types.size()) {
            continue;
        }
        arity_matches.push_back(&candidate);
        int rank = 0;
        for (std::size_t index = 0; index < argument_types.size(); ++index) {
            const int conversion = conversion_rank(candidate.parameters[index], argument_types[index]);
            if (conversion == 1000000) {
                rank = 1000000;
                break;
            }
            rank += conversion;
        }
        if (rank < best_rank) {
            best_rank = rank;
            best_count = 1;
        } else if (rank == best_rank) {
            ++best_count;
        }
    }

    if (arity_matches.empty()) {
        error("function '" + name + "' called with incorrect number of arguments");
    } else if (best_rank == 1000000) {
        error("function '" + name + "' has no matching overload");
    } else if (best_count > 1) {
        error("ambiguous call to overloaded function '" + name + "'");
    }
}

void validate_switch(const AstNode *node, VariableTable variables,
                     const AggregateTable &aggregates, const FunctionTable &functions,
                     Context context)
{
    if (!node || node->children.size() < 2) {
        return;
    }
    validate(node->children[0], variables, aggregates, functions, context);
    ++context.switch_depth;
    std::set<std::string> cases;
    bool has_default = false;
    for (const AstNode *section : node->children[1]->children) {
        if (section->children.empty()) {
            continue;
        }
        const AstNode *label = section->children[0];
        if (label->kind == "Case") {
            const std::string key = label->children.empty() ? "" : expression_key(label->children[0]);
            if (!cases.insert(key).second) {
                error("duplicate case value");
            }
        } else if (label->kind == "Default") {
            if (has_default) {
                error("duplicate default label");
            }
            has_default = true;
        }
        for (const AstNode *child : section->children) {
            validate(child, variables, aggregates, functions, context);
        }
    }
}

bool validate(const AstNode *node, VariableTable variables, const AggregateTable &aggregates,
              const FunctionTable &functions, Context context)
{
    if (!node) {
        return true;
    }

    if (node->kind == "ClassDefinition" || node->kind == "StructDefinition") {
        const AstNode *members = node->kind == "ClassDefinition"
            ? (node->children.size() > 1 ? node->children[1] : nullptr)
            : (!node->children.empty() ? node->children[0] : nullptr);
        if (members) {
            Context member_context = context;
            member_context.current_class = node->value;
            for (const AstNode *member : members->children) {
                if (member->kind == "FunctionDefinition" || member->kind == "ConstructorDefinition") {
                    validate(member, VariableTable{}, aggregates, functions, member_context);
                }
            }
        }
        return true;
    }

    if (node->kind == "FunctionDefinition" || node->kind == "ConstructorDefinition") {
        VariableTable locals;
        record_parameters(parameters_of(node), locals);
        std::set<std::string> labels;
        std::set<std::string> gotos;
        collect_labels(body_of(node), labels, gotos);
        for (const std::string &target : gotos) {
            if (labels.count(target) == 0) {
                error("goto target '" + target + "' is undefined");
            }
        }

        Context function_context = context;
        function_context.expected_return = return_type_of(node);
        validate(body_of(node), locals, aggregates, functions, function_context);
        return true;
    }

    if (node->kind == "List") {
        VariableTable list_variables = variables;
        for (const AstNode *child : node->children) {
            validate(child, list_variables, aggregates, functions, context);
            if (child->kind == "Declaration") {
                record_declaration(child, list_variables);
            }
        }
        return true;
    }

    if (node->kind == "Declaration") {
        validate_declaration(node, variables, aggregates, functions, context);
        return true;
    }

    if (node->kind == "Switch") {
        validate_switch(node, variables, aggregates, functions, context);
        return true;
    }

    if (node->kind == "While" || node->kind == "DoWhile" || node->kind == "For") {
        Context loop_context = context;
        ++loop_context.loop_depth;
        for (const AstNode *child : node->children) {
            validate(child, variables, aggregates, functions, loop_context);
        }
        return true;
    }

    if (node->kind == "Break" && context.loop_depth == 0 && context.switch_depth == 0) {
        error("break statement is not inside a loop or switch");
    }
    if (node->kind == "Continue" && context.loop_depth == 0) {
        error("continue statement is not inside a loop");
    }
    if (node->kind == "This" && context.current_class.empty()) {
        error("'this' used outside a class member function");
    }

    if (node->kind == "Delete" && node->children.size() > 1) {
        const TypeInfo operand = infer_type(node->children[1], variables, aggregates, functions, context);
        if (operand.pointer_depth == 0) {
            error("delete requires a pointer operand");
        }
    }

    if (node->kind == "MemberAccess" && !node->children.empty()) {
        const TypeInfo object = infer_type(node->children[0], variables, aggregates, functions, context);
        const std::string member = node->value.rfind("->", 0) == 0
            ? node->value.substr(2)
            : node->value.substr(1);
        if (!object.base.empty()) {
            const MemberInfo *info = find_member(aggregates, object.base, member);
            if (!info) {
                error("member '" + member + "' is not defined in '" + object.base + "'");
            } else if (info->access == "private" && context.current_class != object.base) {
                error("member '" + member + "' of class '" + object.base + "' is private");
            }
        }
    }

    if (node->kind == "Call") {
        validate_call(node, variables, aggregates, functions, context);
    }

    if (node->kind == "Return" && !node->children.empty()) {
        const TypeInfo actual = infer_type(node->children[0], variables, aggregates, functions, context);
        if (!context.expected_return.base.empty() && !compatible(context.expected_return, actual)) {
            error("return type '" + actual.base + "' does not match function return type '" +
                  context.expected_return.base + "'");
        }
    }

    if (node->kind == "Assignment" && node->children.size() > 1) {
        const TypeInfo target = infer_type(node->children[0], variables, aggregates, functions, context);
        const TypeInfo source = infer_type(node->children[1], variables, aggregates, functions, context);
        if (target.pointer_depth > 0 && source.pointer_depth == 0 && !source.base.empty()) {
            error("cannot assign a non-pointer value to a pointer");
        }
    }

    for (const AstNode *child : node->children) {
        validate(child, variables, aggregates, functions, context);
    }
    return true;
}

TypeInfo infer_type(const AstNode *node, const VariableTable &variables,
                    const AggregateTable &aggregates, const FunctionTable &functions,
                    const Context &context)
{
    if (!node) {
        return {};
    }
    if (node->kind == "IntegerLiteral") return {"int"};
    // Unsuffixed decimal literals have type double in C/C++.
    if (node->kind == "FloatLiteral") return {"double"};
    if (node->kind == "CharLiteral") return {"char"};
    if (node->kind == "StringLiteral") return {"string"};
    if (node->kind == "BooleanLiteral") return {"bool"};
    if (node->kind == "This") return {context.current_class, 1};
    if (node->kind == "Identifier") {
        const auto variable = variables.find(node->value);
        if (variable == variables.end()) {
            return {};
        }
        TypeInfo type = variable->second;
        // In an expression, an array name decays to a pointer to its first
        // element (for example: int *p = values;).
        if (type.is_array) {
            ++type.pointer_depth;
            --type.array_dimensions;
            type.is_array = type.array_dimensions > 0;
        }
        return type;
    }
    if (node->kind == "Unary" && !node->children.empty()) {
        TypeInfo operand = infer_type(node->children[0], variables, aggregates, functions, context);
        if (node->value == "&") ++operand.pointer_depth;
        if (node->value == "*" && operand.pointer_depth > 0) --operand.pointer_depth;
        return operand;
    }
    if (node->kind == "ArrayAccess" && !node->children.empty()) {
        // Indexing an array produces its element type, rather than the
        // pointer type produced by array-to-pointer decay.
        if (node->children[0]->kind == "Identifier") {
            const auto variable = variables.find(node->children[0]->value);
            if (variable != variables.end() && variable->second.is_array) {
                TypeInfo element = variable->second;
                --element.array_dimensions;
                element.is_array = element.array_dimensions > 0;
                if (element.array_dimensions > 0) {
                    ++element.pointer_depth;
                }
                return element;
            }
        }
        TypeInfo array = infer_type(node->children[0], variables, aggregates, functions, context);
        if (array.pointer_depth > 0) {
            --array.pointer_depth;
        }
        array.is_array = false;
        if (array.array_dimensions > 0) {
            --array.array_dimensions;
            array.is_array = array.array_dimensions > 0;
        }
        return array;
    }
    if (node->kind == "MemberAccess" && !node->children.empty()) {
        const TypeInfo object = infer_type(node->children[0], variables, aggregates, functions, context);
        const std::string member = node->value.rfind("->", 0) == 0
            ? node->value.substr(2)
            : node->value.substr(1);
        if (const MemberInfo *info = find_member(aggregates, object.base, member)) {
            return info->type;
        }
        return {};
    }
    if (node->kind == "Call" && !node->children.empty() && node->children[0]->kind == "Identifier") {
        const auto overloads = functions.find(node->children[0]->value);
        if (overloads != functions.end() && !overloads->second.empty()) {
            return overloads->second.front().return_type;
        }
    }
    if (node->kind == "Binary" || node->kind == "Conditional" || node->kind == "Prefix" ||
        node->kind == "Postfix" || node->kind == "Sizeof" || node->kind == "SizeofType") {
        return {"int"};
    }
    return {};
}
} // namespace

bool semantic_validate(const AstNode *root)
{
    g_errors = 0;
    AggregateTable aggregates;
    FunctionTable functions;
    std::map<std::string, TypeInfo> typedefs;

    collect_aggregates(root, aggregates);
    collect_functions(root, functions);
    collect_typedefs(root, typedefs);

    for (const auto &entry : aggregates) {
        for (const std::string &base : entry.second.bases) {
            if (aggregates.count(base) == 0) {
                error("base class '" + base + "' of '" + entry.first + "' is undefined");
            }
        }
    }

    validate(root, VariableTable{}, aggregates, functions, Context{});
    return g_errors == 0;
}
