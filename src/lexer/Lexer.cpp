#include "Lexer.h"
#include <cctype>

namespace quill {

Lexer::Lexer(std::string source) : source_(std::move(source)) {
    // Read the very first character so we are ready to go
    advance(); 
}

void Lexer::advance() {
    if (read_position_ >= source_.length()) {
        current_char_ = '\0'; // End of file/string
    } else {
        current_char_ = source_[read_position_];
    }
    position_ = read_position_;
    read_position_++;
}

void Lexer::skipWhitespace() {
    while (current_char_ == ' ' || current_char_ == '\t' || 
           current_char_ == '\n' || current_char_ == '\r') {
        advance();
    }
}

bool Lexer::isLetter(char ch) {
    // We include '_' and '.' so we can handle 'users.id'
    return (ch >= 'a' && ch <= 'z') || 
           (ch >= 'A' && ch <= 'Z') || 
           ch == '_' || ch == '.';
}

bool Lexer::isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

std::string Lexer::readIdentifier() {
    size_t start_pos = position_;
    while (isLetter(current_char_)) {
        advance();
    }
    return source_.substr(start_pos, position_ - start_pos);
}

std::string Lexer::readNumber() {
    size_t start_pos = position_;
    while (isDigit(current_char_)) {
        advance();
    }
    return source_.substr(start_pos, position_ - start_pos);
}

TokenType Lexer::lookupIdentifier(const std::string& ident) {
    std::string upper_ident = ident;
    for (char& c : upper_ident) c = std::toupper(c);
    
    if (upper_ident == "SELECT") return TokenType::SELECT;
    if (upper_ident == "FROM") return TokenType::FROM;
    if (upper_ident == "WHERE") return TokenType::WHERE;
    if (upper_ident == "JOIN") return TokenType::JOIN;
    if (upper_ident == "ON") return TokenType::ON;
    if (upper_ident == "GROUP") return TokenType::GROUP;
    if (upper_ident == "BY") return TokenType::BY;
    
    // New Keyword :
    if (upper_ident == "EXPLAIN") return TokenType::EXPLAIN;
    
    return TokenType::IDENTIFIER;
}

Token Lexer::nextToken() {
    skipWhitespace();
    
    Token token;
    switch (current_char_) {
        case '=':
            token = {TokenType::EQUALS, "="};
            break;
        case ';':
            token = {TokenType::SEMICOLON, ";"};
            break;
        case '(':
            token = {TokenType::LPAREN, "("};
            break;
        case ')':
            token = {TokenType::RPAREN, ")"};
            break;
        case ',':
            token = {TokenType::COMMA, ","};
            break;
        case '*':
            token = {TokenType::STAR, "*"};
            break;
        case '\0':
            token = {TokenType::END_OF_FILE, ""};
            break;
        default:
            if (isLetter(current_char_)) {
                token.literal = readIdentifier();
                token.type = lookupIdentifier(token.literal);
                return token; // Return early because readIdentifier() already calls advance()
            } else if (isDigit(current_char_)) {
                token.literal = readNumber();
                token.type = TokenType::NUMBER;
                return token; // Return early
            } else {
                token = {TokenType::ILLEGAL, std::string(1, current_char_)};
            }
            break;
    }

    advance();
    return token;
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token t = nextToken();
    
    while (t.type != TokenType::END_OF_FILE) {
        tokens.push_back(t);
        t = nextToken();
    }
    
    tokens.push_back(t); // Don't forget to push the EOF token
    return tokens;
}

} // namespace quill