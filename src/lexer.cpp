#include "lexer.h"
#include <iostream>
#include <stdexcept>

Lexer::Lexer(const std::string& src) : source(src), pos(0), line(1) {}

char Lexer::peek() { return pos < source.size() ? source[pos] : '\0'; }
char Lexer::advance() { return pos < source.size() ? source[pos++] : '\0'; }
bool Lexer::at_end() { return pos >= source.size(); }
bool Lexer::is_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool Lexer::is_digit(char c) { return c >= '0' && c <= '9'; }
bool Lexer::is_alnum(char c) { return is_alpha(c) || is_digit(c); }

void Lexer::skip_whitespace() {
    while (!at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            if (c == '\n') line++;
            advance();
        } else if (c == '/' && peekNext() == '/') { 
            while (peek() != '\n' && !at_end()) advance();
        } else {
            break;
        }
    }
}

char Lexer::peekNext() { return pos + 1 < source.size() ? source[pos + 1] : '\0'; } 

std::string Lexer::scan_identifier() {
    std::string value(1, source[pos - 1]);
    while (is_alnum(peek())) value += advance();
    if (value == "blueprint") return "blueprint";
    if (value == "define") return "define";
    if (value == "instance") return "instance";
    if (value == "var") return "var";
    if (value == "integer") return "integer";
    if (value == "check_if") return "check_if";
    if (value == "otherwise") return "otherwise";
    if (value == "repeat_while") return "repeat_while";
    if (value == "lets_print") return "lets_print";
    if (value == "scanning_user_input") return "scanning_user_input";
    if (value == "yield") return "yield";
    if (value == "let") return "let";              // Added
    if (value == "const") return "const";          // Added
    if (value == "if") return "if";                // Added
    if (value == "true") return "true";            // Added
    if (value == "false") return "false";          // Added
    if (value == "else_when") return "else_when";  // Added
    return value;
}

std::string Lexer::scan_number() {
    std::string value(1, source[pos - 1]);
    while (is_digit(peek())) value += advance();
    return value;
}

std::string Lexer::scan_string() {
    std::string value;
    while (peek() != '"' && !at_end()) {
        if (peek() == '\n') line++;
        value += advance();
    }
    if (at_end()) throw std::runtime_error("Unterminated string at line " + std::to_string(line));
    advance(); // Skip closing "
    return value;
}

Token Lexer::next_token() {
    skip_whitespace();
    if (at_end()) return {TOK_EOF, "", line};
    size_t start_pos = pos; // Capture start position
    char c = advance();
    switch (c) {
        case '+': return {TOK_PLUS, "+", line};
        case '-': return {TOK_MINUS, "-", line};
        case '(': return {TOK_LPAREN, "(", line};
        case ')': return {TOK_RPAREN, ")", line};
        case '{': return {TOK_LBRACE, "{", line};
        case '}': return {TOK_RBRACE, "}", line};
        case ';': return {TOK_SEMICOLON, ";", line};
        case ',': return {TOK_COMMA, ",", line};
        case '.': return {TOK_DOT, ".", line};
        
        case '=':
            if (peek() == '=') {
                advance();
                return {TOK_EQ, "==", line};
            }
            break;

        case ':': 
            if (peek() == '=') {
                advance();
                return {TOK_ASSIGN, ":=", line};
            }
            break;
        case '<':
            if (peek() == '=') {
                advance();
                return {TOK_LTE, "<=", line};
            }
            return {TOK_LT, "<", line}; // Fixed
        case '!':
            if (peek() == '<') {
                advance();
                return {TOK_NOT_LT, "!<", line};
            }
            break;
        case '>': return {TOK_GT, ">", line};
        case '"': return {TOK_STRING, scan_string(), line};
        default:
            if (is_alpha(c)) {
                std::string id = scan_identifier();
                if (id == "blueprint") return {TOK_BLUEPRINT, id, line};
                if (id == "define") return {TOK_DEFINE, id, line};
                if (id == "instance") return {TOK_INSTANCE, id, line};
                if (id == "var") return {TOK_VAR, id, line};
                if (id == "integer") return {TOK_INTEGER, id, line};
                if (id == "check_if") return {TOK_CHECK_IF, id, line};
                if (id == "otherwise") return {TOK_OTHERWISE, id, line};
                if (id == "repeat_while") return {TOK_REPEAT_WHILE, id, line};
                if (id == "lets_print") return {TOK_LETS_PRINT, id, line};
                if (id == "scanning_user_input") return {TOK_SCANNING_USER_INPUT, id, line};
                if (id == "yield") return {TOK_YIELD, id, line};
                if (id == "let") return {TOK_LET, id, line};              // Added
                if (id == "const") return {TOK_CONST, id, line};          // Added
                if (id == "if") return {TOK_IF, id, line};                // Added
                if (id == "true") return {TOK_TRUE, id, line};            // Added
                if (id == "false") return {TOK_FALSE, id, line};          // Added
                if (id == "else_when") return {TOK_ELSE_WHEN, id, line};  // Added
                return {TOK_IDENTIFIER, id, line};
            }
            if (is_digit(c)) return {TOK_NUMBER, scan_number(), line};
            throw std::runtime_error("Unexpected character '" + std::string(1, c) + "' at line " + std::to_string(line));
    }
    throw std::runtime_error("Unhandled token at line " + std::to_string(line)); // Catch unhandled cases
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!at_end()) {
        Token t = next_token();
        if (t.type == TOK_EOF) break;
        std::cout << "Advancing to pos " << pos - t.value.size() << ", token: " << t.value << "\n"; // Fix pos
        tokens.push_back(t);
    }
    tokens.push_back({TOK_EOF, "", line});
    return tokens;
}