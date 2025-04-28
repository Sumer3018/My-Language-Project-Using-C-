#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <vector>


class Parser {
public:
    Parser(const std::vector<Token>& t);
    std::unique_ptr<ASTNode> parse();

private:
    std::vector<Token> tokens;
    size_t pos;

    Token peek();
    Token advance();
    bool match(TokenType type);
    bool at_end();
    Token peek_next();
    int get_precedence(TokenType type);
    Token expect(TokenType type, const std::string& msg);

    std::unique_ptr<ASTNode> statement();
    std::unique_ptr<ASTNode> blueprint();
    std::unique_ptr<ASTNode> var_decl();
    std::unique_ptr<ASTNode> let_const_decl(); // Added missing declaration
    std::unique_ptr<ASTNode> function();
    std::unique_ptr<ASTNode> if_stmt();
    std::unique_ptr<ASTNode> while_stmt();
    std::unique_ptr<ASTNode> print_stmt();
    std::unique_ptr<ASTNode> input_stmt();
    std::unique_ptr<ASTNode> yield_stmt();
    std::unique_ptr<ASTNode> instance_stmt();
    std::unique_ptr<ASTNode> assignment(); // Keep this for compatibility
    std::unique_ptr<ASTNode> parse_assignment(const Token& id); // Added declaration
    std::unique_ptr<ASTNode> expression();
    std::unique_ptr<ASTNode> term();
    std::unique_ptr<ASTNode> factor();

    // Placeholder declarations
    std::unique_ptr<ASTNode> return_stmt();
    std::unique_ptr<ASTNode> logical();
    std::unique_ptr<ASTNode> comparison();
    std::unique_ptr<ASTNode> primary();
};

#endif