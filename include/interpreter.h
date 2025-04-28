#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

struct Value {
    enum class Type { None, Int, String, Instance } type;
    int int_val;
    std::string str_val;
    std::unique_ptr<std::unordered_map<std::string, Value>> instance_fields;
    std::string blueprint_name;

    Value() : type(Type::None), int_val(0), instance_fields(nullptr) {}
    explicit Value(int v) : type(Type::Int), int_val(v), instance_fields(nullptr) {}
    explicit Value(const std::string& v) : type(Type::String), str_val(v), instance_fields(nullptr) {}
    Value(const std::string& bn, const std::unordered_map<std::string, Value>& fields)
        : type(Type::Instance), blueprint_name(bn), instance_fields(new std::unordered_map<std::string, Value>(fields)) {}
    Value(const Value& other) 
        : type(other.type), int_val(other.int_val), str_val(other.str_val), blueprint_name(other.blueprint_name) {
        if (other.instance_fields) {
            instance_fields.reset(new std::unordered_map<std::string, Value>(*other.instance_fields));
        } else {
            instance_fields = nullptr;
        }
    }
    Value& operator=(const Value& other) {
        if (this != &other) {
            type = other.type;
            int_val = other.int_val;
            str_val = other.str_val;
            blueprint_name = other.blueprint_name;
            if (other.instance_fields) {
                instance_fields.reset(new std::unordered_map<std::string, Value>(*other.instance_fields));
            } else {
                instance_fields.reset();
            }
        }
        return *this;
    }
    ~Value() = default;

    std::string as_string() const {
        if (type == Type::Int) return std::to_string(int_val);
        if (type == Type::String) return str_val;
        return "";
    }
    int as_int() const { return type == Type::Int ? int_val : std::stoi(str_val); }
};

class InterpreterVisitor : public ASTVisitor {
public:
    struct RuntimeError : public std::runtime_error {
        RuntimeError(const std::string& msg, int l) : std::runtime_error(msg + " at line " + std::to_string(l)) {}
    };

    struct ReturnValue : public std::runtime_error {
        Value value;
        ReturnValue(const Value& v) : std::runtime_error("Return"), value(v) {}
    };

    void visit(ProgramNode& node) override;
    void visit(BlueprintNode& node) override;
    void visit(VarDeclNode& node) override;
    void visit(FunctionNode& node) override;
    void visit(IfNode& node) override;
    void visit(WhileNode& node) override;
    void visit(PrintNode& node) override;
    void visit(InputNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(IdentifierNode& node) override;
    void visit(NumberNode& node) override;
    void visit(StringNode& node) override;
    void visit(BooleanNode& node) override;
    void visit(AssignmentNode& node) override;
    void visit(CallNode& node) override;
    void visit(YieldNode& node) override;
    void visit(InstanceNode& node) override;
    void visit(LetConstDeclNode& node) override;

    Value evaluate(ASTNode* node);
    Value* current_instance = nullptr;
    Value call_function(const std::string& name, const std::vector<Value>& args = {});
    bool to_bool(const Value& value);

private:
    std::vector<std::unordered_map<std::string, Value>> scopes;
    std::unordered_map<std::string, BlueprintNode*> blueprints;
    std::unordered_map<std::string, FunctionNode*> functions;
    std::string current_scope; // Added to track nested blueprint scope
    Value call_method(const Value& instance, const std::string& method_name, const std::vector<Value>& args = {});
};

#endif