#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

enum TokenType {
    TOK_EOF,
    TOK_VAR,
    TOK_INTEGER,
    TOK_BLUEPRINT,
    TOK_DEFINE,
    TOK_CHECK_IF,
    TOK_OTHERWISE,
    TOK_REPEAT_WHILE,
    TOK_LETS_PRINT,
    TOK_SCANNING_USER_INPUT,
    TOK_YIELD,
    TOK_INSTANCE,
    TOK_IDENTIFIER,
    TOK_NUMBER,
    TOK_STRING,
    TOK_PLUS,
    TOK_MINUS,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMICOLON,
    TOK_ASSIGN,
    TOK_LTE,      // <=
    TOK_NOT_LT,   // !< (unused?)
    TOK_GT,       // >
    TOK_LT,       // Add this for < 
    TOK_DOT,
    TOK_EQ,
    TOK_COMMA,
    TOK_LET,      // Added new token
    TOK_CONST,    // Added new token
    TOK_IF,       // Added new token
    TOK_TRUE,     // Added new token
    TOK_FALSE,    // Added new token
    TOK_ELSE_WHEN // Added new token for "else if"
};

struct Token {
    TokenType type;
    std::string value;
    int line;
};

class Lexer {
public:
    Lexer(const std::string& src);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line;

    char peek();
    char advance();
    bool at_end();
    bool is_alpha(char c);
    bool is_digit(char c);
    bool is_alnum(char c);
    void skip_whitespace();
    char peekNext(); // Should be here
    std::string scan_identifier();
    std::string scan_number();
    std::string scan_string();
    Token next_token();
};

#endif