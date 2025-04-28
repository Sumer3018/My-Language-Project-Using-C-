#include "parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& t) : tokens(t), pos(0) {}

Token Parser::peek()                         { return pos < tokens.size() ? tokens[pos] : Token{TOK_EOF, "", 0};         }
Token Parser::advance()                      { return pos++ < tokens.size() ? tokens[pos - 1] : Token{TOK_EOF, "", 0};   }
bool  Parser::match(TokenType type)          { return peek().type == type;                                               }
bool  Parser::at_end()                       { return match(TOK_EOF);                                                    }
Token Parser::peek_next()                    { return pos + 1 < tokens.size() ? tokens[pos + 1] : Token{TOK_EOF, "", 0}; }
int   Parser::get_precedence(TokenType type) { return 0;                                                                 }

Token Parser::expect(TokenType type, const std::string& msg) {
    if (!match(type)) {
        Token t = peek();
        throw std::runtime_error(msg + " but got '" + t.value + "' at line " + std::to_string(t.line));
    }
    return advance();
}

std::unique_ptr<ASTNode> Parser::parse() {
    auto root = std::unique_ptr<ProgramNode>(new ProgramNode(1));
    while (!match(TOK_EOF)) {
        root->statements.push_back(statement());
    }
    return root;
}

std::unique_ptr<ASTNode> Parser::statement() {
    if (match(TOK_BLUEPRINT))                 return blueprint();
    if (match(TOK_VAR) || match(TOK_INTEGER)) return var_decl();
    if (match(TOK_LET) || match(TOK_CONST))   return let_const_decl();
    if (match(TOK_DEFINE))                    return function();
    if (match(TOK_CHECK_IF) || match(TOK_IF)) return if_stmt();
    if (match(TOK_REPEAT_WHILE))              return while_stmt();
    if (match(TOK_LETS_PRINT))                return print_stmt();
    if (match(TOK_SCANNING_USER_INPUT))       return input_stmt();
    if (match(TOK_YIELD))                     return yield_stmt();
    if (match(TOK_INSTANCE))                  return instance_stmt();
    if (match(TOK_IDENTIFIER)) {
        Token id = advance();
        if (match(TOK_ASSIGN)) {
            return parse_assignment(id);
        } else if (match(TOK_DOT)) {
            advance();
            Token method = expect(
                TOK_IDENTIFIER, 
                "Expected method name after '.'"
            );
            expect(
                TOK_LPAREN, 
                "Expected '(' after method name"
            );
            auto call = std::unique_ptr<CallNode>(new CallNode(id.value + "." + method.value, id.line));
            if (!match(TOK_RPAREN)) {
                do {
                    call->arguments.push_back(expression());
                    if (match(TOK_COMMA)) advance();
                } while (!match(TOK_RPAREN));
                expect(
                    TOK_RPAREN, 
                    "Expected ')' after arguments"
                );
            } else {
                advance();
            }
            expect(
                TOK_SEMICOLON, 
                "Expected ';' after method call"
            );
            return call;
        } else if (match(TOK_LPAREN)) {
            advance();
            auto call = std::unique_ptr<CallNode>(new CallNode(id.value, id.line));
            if (!match(TOK_RPAREN)) {
                do {
                    call->arguments.push_back(expression());
                    if (match(TOK_COMMA)) advance();
                } while (!match(TOK_RPAREN));
                expect(
                    TOK_RPAREN, 
                    "Expected ')' after arguments"
                );
            } else {
                advance();
            }
            expect(
                TOK_SEMICOLON, 
                "Expected ';' after call"
            );
            return call;
        } else {
            auto expr = std::unique_ptr<IdentifierNode>(new IdentifierNode(id.value, id.line));
            if (match(TOK_SEMICOLON)) advance();
            return expr;
        }
    }
    auto expr = expression();
    if (match(TOK_SEMICOLON)) advance();
    return expr;
}

std::unique_ptr<ASTNode> Parser::blueprint() {
    int line = expect(TOK_BLUEPRINT, "Expected 'blueprint'").line;
    Token name = expect(TOK_IDENTIFIER, "Expected blueprint name");
    expect(TOK_LBRACE, "Expected '{'");
    auto node = std::unique_ptr<BlueprintNode>(new BlueprintNode(name.value, line));
    while (!match(TOK_RBRACE)) {
        if (match(TOK_DEFINE)) {
            node->body.push_back(function());
        } else if (match(TOK_BLUEPRINT)) {
            node->body.push_back(blueprint());
        } else {
            throw std::runtime_error("Expected function or blueprint definition in blueprint at line " + std::to_string(peek().line));
        }
    }
    expect(TOK_RBRACE, "Expected '}'");
    return node;
}

std::unique_ptr<ASTNode> Parser::var_decl() {
    Token decl = match(TOK_VAR) ? advance() : expect(TOK_INTEGER, "Expected 'var' or 'integer'");
    Token id = expect(TOK_IDENTIFIER, "Expected variable name");
    expect(TOK_ASSIGN, "Expected ':='");
    auto expr = expression();
    expect(TOK_SEMICOLON, "Expected ';'");
    auto node = std::unique_ptr<VarDeclNode>(new VarDeclNode(decl.value, id.value, decl.line));
    node->initializer = std::move(expr);
    return node;
}

std::unique_ptr<ASTNode> Parser::let_const_decl() {
    bool is_const = match(TOK_CONST);
    int line = advance().line; // Consume LET or CONST
    Token id = expect(TOK_IDENTIFIER, "Expected variable name");
    expect(TOK_ASSIGN, "Expected ':='");
    auto expr = expression();
    expect(TOK_SEMICOLON, "Expected ';'");
    auto node = std::unique_ptr<LetConstDeclNode>(new LetConstDeclNode(is_const, id.value, line));
    node->initializer = std::move(expr);
    return node;
}

std::unique_ptr<ASTNode> Parser::function() {
    int line = expect(TOK_DEFINE, "Expected 'define'").line;
    Token name = expect(TOK_IDENTIFIER, "Expected function name");
    auto node = std::unique_ptr<FunctionNode>(new FunctionNode(name.value, line));
    expect(TOK_LPAREN, "Expected '(' after function name");
    if (!match(TOK_RPAREN)) {
        do {
            Token param = expect(TOK_IDENTIFIER, "Expected parameter name");
            node->parameters.push_back(param.value);
            if (match(TOK_COMMA)) advance();
        } while (!match(TOK_RPAREN));
        expect(TOK_RPAREN, "Expected ')' after parameters");
    } else {
        advance();
    }
    expect(TOK_LBRACE, "Expected '{' after function definition");
    while (!match(TOK_RBRACE)) {
        node->body.push_back(statement());
    }
    expect(TOK_RBRACE, "Expected '}'");
    return node;
}

std::unique_ptr<ASTNode> Parser::if_stmt() {
    int line = (match(TOK_CHECK_IF) ? expect(TOK_CHECK_IF, "Expected 'check_if'") : expect(TOK_IF, "Expected 'if'")).line;
    expect(TOK_LPAREN, "Expected '(' before condition");
    auto condition = expression();
    expect(TOK_RPAREN, "Expected ')' after condition");
    expect(TOK_LBRACE, "Expected '{'");
    auto then_block = std::unique_ptr<ProgramNode>(new ProgramNode(line));
    while (!match(TOK_RBRACE)) {
        then_block->statements.push_back(statement());
    }
    expect(TOK_RBRACE, "Expected '}'");
    
    auto node = std::unique_ptr<IfNode>(new IfNode(line));
    node->condition = std::move(condition);
    node->then_block = std::move(then_block);
    
    // Handle else_when (else if) blocks
    while (match(TOK_ELSE_WHEN)) {
        advance(); // Consume ELSE_WHEN
        expect(TOK_LPAREN, "Expected '(' before else_when condition");
        auto else_if_condition = expression();
        expect(TOK_RPAREN, "Expected ')' after else_when condition");
        expect(TOK_LBRACE, "Expected '{'");
        auto else_if_block = std::unique_ptr<ProgramNode>(new ProgramNode(line));
        while (!match(TOK_RBRACE)) {
            else_if_block->statements.push_back(statement());
        }
        expect(TOK_RBRACE, "Expected '}'");
        node->else_if_blocks.push_back({std::move(else_if_condition), std::move(else_if_block)});
    }
    
    // Handle else block
    if (match(TOK_OTHERWISE)) {
        advance();
        expect(TOK_LBRACE, "Expected '{'");
        auto else_block = std::unique_ptr<ProgramNode>(new ProgramNode(line));
        while (!match(TOK_RBRACE)) {
            else_block->statements.push_back(statement());
        }
        expect(TOK_RBRACE, "Expected '}'");
        node->else_block = std::move(else_block);
    }
    
    return node;
}

std::unique_ptr<ASTNode> Parser::while_stmt() {
    int line = expect(TOK_REPEAT_WHILE, "Expected 'repeat_while'").line;
    expect(TOK_LPAREN, "Expected '(' before condition");
    auto condition = expression();
    expect(TOK_RPAREN, "Expected ')' after condition");
    expect(TOK_LBRACE, "Expected '{'");
    auto body = std::unique_ptr<ProgramNode>(new ProgramNode(line));
    while (!match(TOK_RBRACE)) {
        body->statements.push_back(statement());
    }
    expect(TOK_RBRACE, "Expected '}'");
    auto node = std::unique_ptr<WhileNode>(new WhileNode(line));
    node->condition = std::move(condition);
    node->body = std::move(body);
    return node;
}

std::unique_ptr<ASTNode> Parser::print_stmt() {
    int line = expect(TOK_LETS_PRINT, "Expected 'lets_print'").line;
    expect(TOK_LBRACE, "Expected '{' before expression");
    auto expr = expression(); // Allow full expressions, including method calls
    expect(TOK_RBRACE, "Expected '}' after expression");
    if (match(TOK_SEMICOLON)) advance();
    auto node = std::unique_ptr<PrintNode>(new PrintNode(line));
    node->expression = std::move(expr);
    return node;
}

std::unique_ptr<ASTNode> Parser::input_stmt() {
    int line = expect(TOK_SCANNING_USER_INPUT, "Expected 'scanning_user_input'").line;
    expect(TOK_LBRACE, "Expected '{'");
    Token type = peek();
    if (match(TOK_INTEGER) || match(TOK_IDENTIFIER)) {
        advance();
    } else {
        throw std::runtime_error("Expected input type but got '" + type.value + "' at line " + std::to_string(type.line));
    }
    expect(TOK_RBRACE, "Expected '}'");
    expect(TOK_SEMICOLON, "Expected ';'");
    return std::unique_ptr<InputNode>(new InputNode(type.value, line));
}

std::unique_ptr<ASTNode> Parser::yield_stmt() {
    int line = expect(TOK_YIELD, "Expected 'yield'").line;
    auto expr = expression();
    expect(TOK_SEMICOLON, "Expected ';' after yield");
    auto node = std::unique_ptr<YieldNode>(new YieldNode(line));
    node->expression = std::move(expr);
    return node;
}

std::unique_ptr<ASTNode> Parser::instance_stmt() {
    int line = expect(TOK_INSTANCE, "Expected 'instance'").line;
    Token blueprint = expect(TOK_IDENTIFIER, "Expected blueprint name");
    Token name = expect(TOK_IDENTIFIER, "Expected instance name");
    expect(TOK_SEMICOLON, "Expected ';'");
    return std::unique_ptr<InstanceNode>(new InstanceNode(blueprint.value, name.value, line));
}

std::unique_ptr<ASTNode> Parser::assignment() {
    Token id = expect(TOK_IDENTIFIER, "Expected identifier");
    expect(TOK_ASSIGN, "Expected ':='");
    auto value = expression();
    expect(TOK_SEMICOLON, "Expected ';'");
    auto node = std::unique_ptr<AssignmentNode>(new AssignmentNode(id.value, id.line));
    node->value = std::move(value);
    return node;
}

std::unique_ptr<ASTNode> Parser::parse_assignment(const Token& id) {
    expect(TOK_ASSIGN, "Expected ':=' after identifier");
    auto value = expression();
    expect(TOK_SEMICOLON, "Expected ';' after assignment");
    auto node = std::unique_ptr<AssignmentNode>(new AssignmentNode(id.value, id.line));
    node->value = std::move(value);
    return node;
}

std::unique_ptr<ASTNode> Parser::expression() {
    auto left = term();
    while (match(TOK_PLUS) || match(TOK_MINUS) || match(TOK_LTE) || match(TOK_NOT_LT) || match(TOK_GT) || match(TOK_LT) || match(TOK_EQ)) {
        Token op = advance();
        auto right = term();
        auto bin_op = std::unique_ptr<BinaryOpNode>(new BinaryOpNode(op.value, op.line));
        bin_op->left = std::move(left);
        bin_op->right = std::move(right);
        left = std::move(bin_op);
    }
    return left;
}

std::unique_ptr<ASTNode> Parser::term() {
    return factor();
}

std::unique_ptr<ASTNode> Parser::factor() {
    if (match(TOK_NUMBER)) {
        Token t = advance();
        return std::unique_ptr<NumberNode>(new NumberNode(t.value, t.line));
    }
    if (match(TOK_STRING)) {
        Token t = advance();
        return std::unique_ptr<StringNode>(new StringNode(t.value, t.line));
    }
    if (match(TOK_TRUE) || match(TOK_FALSE)) {
        Token t = advance();
        return std::unique_ptr<BooleanNode>(new BooleanNode(t.type == TOK_TRUE, t.line));
    }
    if (match(TOK_LPAREN)) {
        advance();
        auto expr = expression();
        expect(TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }
    if (match(TOK_IDENTIFIER)) {
        Token id = advance();
        if (match(TOK_DOT)) {
            advance();
            Token method = expect(TOK_IDENTIFIER, "Expected method name after '.'");
            expect(TOK_LPAREN, "Expected '(' after method name");
            auto call = std::unique_ptr<CallNode>(new CallNode(id.value + "." + method.value, id.line));
            if (!match(TOK_RPAREN)) {
                do {
                    call->arguments.push_back(expression());
                    if (match(TOK_COMMA)) advance();
                } while (!match(TOK_RPAREN));
            }
            expect(TOK_RPAREN, "Expected ')' after arguments");
            return call;
        } else if (match(TOK_LPAREN)) {
            advance();
            auto call = std::unique_ptr<CallNode>(new CallNode(id.value, id.line));
            if (!match(TOK_RPAREN)) {
                do {
                    call->arguments.push_back(expression());
                    if (match(TOK_COMMA)) advance();
                } while (!match(TOK_RPAREN));
            }
            expect(TOK_RPAREN, "Expected ')' after arguments");
            return call;
        }
        return std::unique_ptr<IdentifierNode>(new IdentifierNode(id.value, id.line));
    }
    if (match(TOK_SCANNING_USER_INPUT)) {
        int line = expect(TOK_SCANNING_USER_INPUT, "Expected 'scanning_user_input'").line;
        expect(TOK_LBRACE, "Expected '{'");
        Token type = peek();
        if (match(TOK_INTEGER) || match(TOK_IDENTIFIER)) {
            advance();
        } else {
            throw std::runtime_error("Expected input type but got '" + type.value + "' at line " + std::to_string(type.line));
        }
        expect(TOK_RBRACE, "Expected '}'");
        return std::unique_ptr<InputNode>(new InputNode(type.value, line));
    }
    
    throw std::runtime_error("Unexpected token '" + peek().value + "' at line " + std::to_string(peek().line));
}