#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    virtual void visit(class ProgramNode& node) = 0;
    virtual void visit(class BlueprintNode& node) = 0;
    virtual void visit(class VarDeclNode& node) = 0;
    virtual void visit(class FunctionNode& node) = 0;
    virtual void visit(class IfNode& node) = 0;
    virtual void visit(class WhileNode& node) = 0;
    virtual void visit(class PrintNode& node) = 0;
    virtual void visit(class InputNode& node) = 0;
    virtual void visit(class BinaryOpNode& node) = 0;
    virtual void visit(class IdentifierNode& node) = 0;
    virtual void visit(class NumberNode& node) = 0;
    virtual void visit(class StringNode& node) = 0;
    virtual void visit(class BooleanNode& node) = 0;       // Added
    virtual void visit(class AssignmentNode& node) = 0;
    virtual void visit(class CallNode& node) = 0;
    virtual void visit(class YieldNode& node) = 0;
    virtual void visit(class InstanceNode& node) = 0;
    virtual void visit(class LetConstDeclNode& node) = 0;  // Added
};

class ASTNode {
public:
    int line;
    explicit ASTNode(int l) : line(l) {}
    virtual ~ASTNode() = default;
    virtual void accept(ASTVisitor& visitor) = 0;
};

class ProgramNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;
    explicit ProgramNode(int l) : ASTNode(l) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BlueprintNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> body;
    bool is_abstract = false; // For future abstraction support
    BlueprintNode(const std::string& n, int l) : ASTNode(l), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class VarDeclNode : public ASTNode {
public:
    std::string type; // e.g., "integer", "var" (string), later "real", "truth"
    std::string name;
    std::unique_ptr<ASTNode> initializer;
    bool is_hidden = false; // For encapsulation (private)
    VarDeclNode(const std::string& t, const std::string& n, int l) : ASTNode(l), type(t), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Added new node for let/const declarations
class LetConstDeclNode : public ASTNode {
public:
    bool is_const;
    std::string name;
    std::unique_ptr<ASTNode> initializer;
    LetConstDeclNode(bool is_c, const std::string& n, int l) : ASTNode(l), is_const(is_c), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class YieldNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    YieldNode(int l) : ASTNode(l) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class FunctionNode : public ASTNode {
public:
    std::string name;
    std::vector<std::string> parameters; // Added: Parameter names (e.g., "name" in greet(name))
    std::vector<std::unique_ptr<ASTNode>> body;
    bool is_hidden = false; // For encapsulation (private)
    FunctionNode(const std::string& n, int l) : ASTNode(l), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class IfNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ProgramNode> then_block;
    std::vector<std::pair<std::unique_ptr<ASTNode>, std::unique_ptr<ProgramNode>>> else_if_blocks; // Added for else_when
    std::unique_ptr<ProgramNode> else_block;
    explicit IfNode(int l) : ASTNode(l) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class WhileNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ProgramNode> body;
    explicit WhileNode(int l) : ASTNode(l) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class PrintNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expression;
    explicit PrintNode(int l) : ASTNode(l) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class InputNode : public ASTNode {
public:
    std::string type;
    InputNode(const std::string& t, int l) : ASTNode(l), type(t) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class BinaryOpNode : public ASTNode {
public:
    std::string op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;
    BinaryOpNode(const std::string& o, int l) : ASTNode(l), op(o) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    IdentifierNode(const std::string& n, int l) : ASTNode(l), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class NumberNode : public ASTNode {
public:
    int value; // Later expand to float for "real"
    NumberNode(const std::string& v, int l) : ASTNode(l), value(std::stoi(v)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class StringNode : public ASTNode {
public:
    std::string value;
    StringNode(const std::string& v, int l) : ASTNode(l), value(v) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

// Added for boolean literals (true/false)
class BooleanNode : public ASTNode {
public:
    bool value;
    BooleanNode(bool v, int l) : ASTNode(l), value(v) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class AssignmentNode : public ASTNode {
public:
    std::string name;
    std::unique_ptr<ASTNode> value;
    AssignmentNode(const std::string& n, int l) : ASTNode(l), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class CallNode : public ASTNode {
public:
    std::string name;
    std::vector<std::unique_ptr<ASTNode>> arguments; // Added: Argument expressions (e.g., "Bob" in p.greet("Bob"))
    CallNode(const std::string& n, int l) : ASTNode(l), name(n) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class InstanceNode : public ASTNode {
public:
    std::string blueprint_name;
    std::string instance_name;
    InstanceNode(const std::string& bn, const std::string& in, int l) : ASTNode(l), blueprint_name(bn), instance_name(in) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

#endif