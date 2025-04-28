#include "ast.h"
#include <sstream>

std::string to_string(const ASTNode& node) {
    std::ostringstream oss;
    if (const auto* programNode = dynamic_cast<const ProgramNode*>(&node)) {
        oss << "Program(\"\")";
    } else if (const auto* blueprintNode = dynamic_cast<const BlueprintNode*>(&node)) {
        oss << "Blueprint(\"" << blueprintNode->name << "\")";
    } else if (const auto* varDeclNode = dynamic_cast<const VarDeclNode*>(&node)) {
        oss << "VarDecl(\"" << varDeclNode->type << " " << varDeclNode->name << "\")";
    } else if (const auto* functionNode = dynamic_cast<const FunctionNode*>(&node)) {
        oss << "Function(\"" << functionNode->name << " (";
        for (size_t i = 0; i < functionNode->parameters.size(); ++i) {
            oss << functionNode->parameters[i];
            if (i < functionNode->parameters.size() - 1) oss << ", ";
        }
        oss << ")\")";
    } else if (const auto* ifNode = dynamic_cast<const IfNode*>(&node)) {
        oss << "If(\"\")";
    } else if (const auto* whileNode = dynamic_cast<const WhileNode*>(&node)) {
        oss << "While(\"\")";
    } else if (const auto* printNode = dynamic_cast<const PrintNode*>(&node)) {
        oss << "Print(\"\")";
    } else if (const auto* inputNode = dynamic_cast<const InputNode*>(&node)) {
        oss << "Input(\"" << inputNode->type << "\")";
    } else if (const auto* binaryOpNode = dynamic_cast<const BinaryOpNode*>(&node)) {
        oss << "BinaryOp(\"" << binaryOpNode->op << "\")";
    } else if (const auto* identifierNode = dynamic_cast<const IdentifierNode*>(&node)) {
        oss << "Identifier(\"" << identifierNode->name << "\")";
    } else if (const auto* numberNode = dynamic_cast<const NumberNode*>(&node)) {
        oss << "Number(\"" << numberNode->value << "\")";
    } else if (const auto* stringNode = dynamic_cast<const StringNode*>(&node)) {
        oss << "String(\"" << stringNode->value << "\")";
    } else if (const auto* assignmentNode = dynamic_cast<const AssignmentNode*>(&node)) {
        oss << "Assignment(\"" << assignmentNode->name << "\")";
    } else if (const auto* callNode = dynamic_cast<const CallNode*>(&node)) {
        oss << "Call(\"" << callNode->name << " (";
        for (size_t i = 0; i < callNode->arguments.size(); ++i) {
            oss << to_string(*callNode->arguments[i]);
            if (i < callNode->arguments.size() - 1) oss << ", ";
        }
        oss << ")\")";
    } else if (const auto* yieldNode = dynamic_cast<const YieldNode*>(&node)) {
        oss << "Yield(\"\")";
    } else if (const auto* instanceNode = dynamic_cast<const InstanceNode*>(&node)) {
        oss << "Instance(\"" << instanceNode->blueprint_name << " " << instanceNode->instance_name << "\")";
    }
    return oss.str();
}