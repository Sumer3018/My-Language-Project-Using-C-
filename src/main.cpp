#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <fstream>
#include <iostream>

class PrintVisitor : public ASTVisitor {
private:
    int indent = 0;

    void print_indent() { for (int i = 0; i < indent; ++i) std::cout << "  "; }
    void print_node(const std::string& type, const std::string& value = "") {
        print_indent();
        std::cout << type << "(\"" << value << "\")" << std::endl;
    }

public:
    void visit(ProgramNode& node) override {
        print_node("Program");
        indent++;
        for (auto& stmt : node.statements) stmt->accept(*this);
        indent--;
    }
    void visit(BlueprintNode& node) override {
        print_node("Blueprint", node.name);
        indent++;
        for (auto& stmt : node.body) stmt->accept(*this);
        indent--;
    }
    void visit(VarDeclNode& node) override {
        print_node("VarDecl", node.type);
        indent++;
        if (node.initializer) {
            print_indent();
            std::cout << "Identifier(\"" << node.name << "\")" << std::endl;
            node.initializer->accept(*this);
        }
        indent--;
    }
    void visit(LetConstDeclNode& node) override {
        print_node(node.is_const ? "ConstDecl" : "LetDecl", node.name);
        indent++;
        if (node.initializer) {
            node.initializer->accept(*this);
        }
        indent--;
    }
    void visit(FunctionNode& node) override {
        print_node("Function", node.name);
        indent++;
        for (auto& stmt : node.body) stmt->accept(*this);
        indent--;
    }
    void visit(IfNode& node) override {
        print_node("If");
        indent++;
        node.condition->accept(*this);
        print_node("Then");
        indent++;
        node.then_block->accept(*this);
        indent--;
        
        // Handle else-if blocks
        for (auto& else_if : node.else_if_blocks) {
            print_node("ElseWhen");
            indent++;
            else_if.first->accept(*this);
            print_node("Then");
            indent++;
            else_if.second->accept(*this);
            indent--;
            indent--;
        }
        
        if (node.else_block) {
            print_node("Else");
            indent++;
            node.else_block->accept(*this);
            indent--;
        }
        indent--;
    }
    void visit(WhileNode& node) override {
        print_node("While");
        indent++;
        node.condition->accept(*this);
        node.body->accept(*this);
        indent--;
    }
    void visit(PrintNode& node) override {
        print_node("Print");
        indent++;
        node.expression->accept(*this);
        indent--;
    }
    void visit(InputNode& node) override {
        print_node("Input", node.type);
    }
    void visit(BinaryOpNode& node) override {
        print_node("BinaryOp", node.op);
        indent++;
        node.left->accept(*this);
        node.right->accept(*this);
        indent--;
    }
    void visit(IdentifierNode& node) override {
        print_node("Identifier", node.name);
    }
    void visit(NumberNode& node) override {
        print_node("Number", std::to_string(node.value));
    }
    void visit(StringNode& node) override {
        print_node("String", node.value);
    }
    void visit(BooleanNode& node) override {
        print_node("Boolean", node.value ? "true" : "false");
    }
    void visit(AssignmentNode& node) override {
        print_node("Assignment", node.name);
        indent++;
        node.value->accept(*this);
        indent--;
    }
    void visit(CallNode& node) override {
        print_node("Call", node.name);
        indent++;
        for (auto& arg : node.arguments) {
            arg->accept(*this);
        }
        indent--;
    }
    void visit(YieldNode& node) override {
        print_node("Yield");
        indent++;
        node.expression->accept(*this);
        indent--;
    }
    void visit(InstanceNode& node) override {
        print_node("Instance", node.blueprint_name + " " + node.instance_name);
    }
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    std::cout << "Reading file: " << argv[1] << "\n"; // Add this
    std::cout << "Raw source:\n" << source << "\n";   // Add this

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto ast = parser.parse();

    PrintVisitor printer;
    std::cout << "AST:" << std::endl;
    ast->accept(printer);

    std::cout << "\nExecution:" << std::endl;
    InterpreterVisitor interpreter;
    ast->accept(interpreter);

    return 0;
}