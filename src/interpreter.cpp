#include "interpreter.h"
#include <iostream>
#include <stdexcept>

Value InterpreterVisitor::evaluate(ASTNode* node) {
    node->accept(*this);
    return scopes.back().find("__temp__") != scopes.back().end() ? scopes.back()["__temp__"] : Value();
}

bool InterpreterVisitor::to_bool(const Value& value) {
    if (value.type == Value::Type::Int) return value.as_int() != 0;
    if (value.type == Value::Type::String) return !value.as_string().empty();
    return false;
}

Value InterpreterVisitor::call_function(const std::string& name, const std::vector<Value>& args) {
    auto it = functions.find(name);
    if (it == functions.end()) throw RuntimeError("Undefined function " + name, 0);
    if (it->second->parameters.size() != args.size()) {
        throw RuntimeError("Expected " + std::to_string(it->second->parameters.size()) + 
                          " arguments, got " + std::to_string(args.size()), 0);
    }
    scopes.emplace_back();
    for (size_t i = 0; i < args.size(); ++i) {
        scopes.back()[it->second->parameters[i]] = args[i];
    }
    try {
        for (auto& stmt : it->second->body) {
            stmt->accept(*this);
        }
    } catch (const ReturnValue& rv) {
        scopes.pop_back();
        return rv.value;
    }
    scopes.pop_back();
    return Value();
}

Value InterpreterVisitor::call_method(const Value& instance, const std::string& method_name, const std::vector<Value>& args) {
    if (instance.type != Value::Type::Instance) throw RuntimeError("Cannot call method on non-instance", 0);
    auto blueprint_it = blueprints.find(instance.blueprint_name);
    if (blueprint_it == blueprints.end()) throw RuntimeError("Unknown blueprint " + instance.blueprint_name, 0);
    for (auto& stmt : blueprint_it->second->body) {
        if (auto* func = dynamic_cast<FunctionNode*>(stmt.get())) {
            if (func->name == method_name) {
                if (func->parameters.size() != args.size()) {
                    throw RuntimeError("Expected " + std::to_string(func->parameters.size()) + 
                                      " arguments, got " + std::to_string(args.size()), 0);
                }
                scopes.emplace_back(instance.instance_fields ? *instance.instance_fields : std::unordered_map<std::string, Value>());
                for (size_t i = 0; i < args.size(); ++i) {
                    scopes.back()[func->parameters[i]] = args[i];
                }
                std::string old_scope = current_scope;
                current_scope = instance.blueprint_name;
                try {
                    for (auto& body_stmt : func->body) {
                        body_stmt->accept(*this);
                    }
                } catch (const ReturnValue& rv) {
                    current_scope = old_scope;
                    scopes.pop_back();
                    return rv.value;
                }
                current_scope = old_scope;
                scopes.pop_back();
                return Value();
            }
        }
    }
    throw RuntimeError("Method " + method_name + " not found in " + instance.blueprint_name, 0);
}

void InterpreterVisitor::visit(ProgramNode& node) {
    scopes.emplace_back();
    current_scope.clear();
    for (size_t i = 0; i < node.statements.size(); ++i) {
        node.statements[i]->accept(*this);
    }
    scopes.pop_back();
}

void InterpreterVisitor::visit(BlueprintNode& node) {
    std::string full_name = current_scope.empty() ? node.name : current_scope + "." + node.name;
    blueprints[full_name] = &node;
    std::string old_scope = current_scope;
    current_scope = full_name;
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    current_scope = old_scope;
}

void InterpreterVisitor::visit(VarDeclNode& node) {
    Value val = evaluate(node.initializer.get());
    if (node.type == "integer" && val.type != Value::Type::Int) {
        throw RuntimeError("Expected integer for variable " + node.name, node.line);
    }
    scopes.back()[node.name] = val;
}

void InterpreterVisitor::visit(LetConstDeclNode& node) {
    Value val = evaluate(node.initializer.get());
    scopes.back()[node.name] = val;
}

void InterpreterVisitor::visit(FunctionNode& node) {
    std::string full_name = current_scope.empty() ? node.name : current_scope + "." + node.name;
    functions[full_name] = &node;
}

void InterpreterVisitor::visit(IfNode& node) {
    Value cond = evaluate(node.condition.get());
    if (to_bool(cond)) {
        node.then_block->accept(*this);
    } else {
        // Handle else_when (else if) blocks if present
        bool condition_met = false;
        for (auto& else_if_block : node.else_if_blocks) {
            Value else_if_cond = evaluate(else_if_block.first.get());
            if (to_bool(else_if_cond)) {
                else_if_block.second->accept(*this);
                condition_met = true;
                break;
            }
        }
        // If no else_when condition was true and there's an else block, execute it
        if (!condition_met && node.else_block) {
            node.else_block->accept(*this);
        }
    }
}

void InterpreterVisitor::visit(WhileNode& node) {
    while (true) {
        Value cond = evaluate(node.condition.get());
        if (!to_bool(cond)) break;
        for (auto& stmt : node.body->statements) {
            stmt->accept(*this);
        }
    }
}

void InterpreterVisitor::visit(PrintNode& node) {
    Value val = evaluate(node.expression.get());
    if (val.type == Value::Type::String) {
        std::cout << val.as_string() << "\n";
    } else {
        std::cout << val.as_int() << "\n";
    }
}

void InterpreterVisitor::visit(InputNode& node) {
    std::cout << "Enter " << node.type << ": ";
    if (node.type == "integer") {
        int value;
        std::cin >> value;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            throw RuntimeError("Invalid integer input", node.line);
        }
        std::cin.ignore(10000, '\n');
        scopes.back()["__temp__"] = Value(value);
    } else {
        std::string input;
        std::getline(std::cin, input);
        scopes.back()["__temp__"] = Value(input);
    }
}

void InterpreterVisitor::visit(BinaryOpNode& node) {
    Value left = evaluate(node.left.get());
    Value right = evaluate(node.right.get());
    Value result;
    if (node.op == "+") {
        if (left.type == Value::Type::String || right.type == Value::Type::String) {
            result = Value(left.as_string() + right.as_string());
        } else {
            result = Value(left.as_int() + right.as_int());
        }
    } else if (node.op == "-") {
        result = Value(left.as_int() - right.as_int());
    } else if (node.op == "<=") {
        result = Value(left.as_int() <= right.as_int() ? 1 : 0);
    } else if (node.op == "!<") {
        result = Value(left.as_int() >= right.as_int() ? 1 : 0);
    } else if (node.op == ">") {
        result = Value(left.as_int() > right.as_int() ? 1 : 0);
    } 
    else if (node.op == "==") {
    result = Value(left.as_int() == right.as_int() ? 1 : 0);
    }

    else if (node.op == "*") {
        result = Value(left.as_int() * right.as_int());
    } 
    
    else if (node.op == "/") {
        if (right.as_int() == 0) throw RuntimeError("Division by zero", node.line);
        result = Value(left.as_int() / right.as_int());
    } 
    
    else if (node.op == "&&") {
        result = Value(to_bool(left) && to_bool(right) ? 1 : 0);
    }

    else {
        throw RuntimeError("Invalid operation " + node.op, node.line);
    }
    scopes.back()["__temp__"] = result;
}

void InterpreterVisitor::visit(IdentifierNode& node) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto var_it = it->find(node.name);
        if (var_it != it->end()) {
            scopes.back()["__temp__"] = var_it->second;
            return;
        }
    }
    throw RuntimeError("Undefined variable " + node.name, node.line);
}

void InterpreterVisitor::visit(NumberNode& node) {
    scopes.back()["__temp__"] = Value(node.value);
}

void InterpreterVisitor::visit(StringNode& node) {
    scopes.back()["__temp__"] = Value(node.value);
}

void InterpreterVisitor::visit(BooleanNode& node) {
    scopes.back()["__temp__"] = Value(node.value ? 1 : 0);
}

void InterpreterVisitor::visit(AssignmentNode& node) {
    Value val = evaluate(node.value.get());
    scopes.back()[node.name] = val;
}

void InterpreterVisitor::visit(CallNode& node) {
    std::vector<Value> args;
    for (auto& arg : node.arguments) {
        args.push_back(evaluate(arg.get()));
    }
    std::string call_name = node.name;
    size_t dot_pos = call_name.find('.');
    if (dot_pos != std::string::npos) {
        std::string inst_name = call_name.substr(0, dot_pos);
        std::string method_name = call_name.substr(dot_pos + 1);
        for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
            auto inst_it = it->find(inst_name);
            if (inst_it != it->end() && inst_it->second.type == Value::Type::Instance) {
                Value result = call_method(inst_it->second, method_name, args);
                scopes.back()["__temp__"] = result; // Store return value
                return;
            }
        }
        throw RuntimeError("Instance " + inst_name + " not found", node.line);
    }
    Value result = call_function(call_name, args);
    scopes.back()["__temp__"] = result; // Store return value
}

void InterpreterVisitor::visit(YieldNode& node) {
    Value val = evaluate(node.expression.get());
    throw ReturnValue(val);
}

void InterpreterVisitor::visit(InstanceNode& node) {
    std::string blueprint_name = node.blueprint_name;
    std::string scoped_name = current_scope.empty() ? blueprint_name : current_scope + "." + blueprint_name;
    auto it = blueprints.find(scoped_name);
    if (it == blueprints.end()) {
        it = blueprints.find(blueprint_name);
        if (it == blueprints.end()) {
            throw RuntimeError("Blueprint " + blueprint_name + " not defined", node.line);
        }
    }
    std::unordered_map<std::string, Value> fields;
    scopes.back()[node.instance_name] = Value(it->first, fields);
}