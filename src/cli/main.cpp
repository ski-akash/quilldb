#include "lexer/Lexer.h"
#include <iostream>
#include <vector>

std::string tokenTypeToString(quill::TokenType type) {
    switch (type) {
        case quill::TokenType::SELECT: return "SELECT";
        case quill::TokenType::FROM: return "FROM";
        case quill::TokenType::WHERE: return "WHERE";
        case quill::TokenType::IDENTIFIER: return "IDENTIFIER";
        case quill::TokenType::NUMBER: return "NUMBER";
        case quill::TokenType::EQUALS: return "EQUALS";
        case quill::TokenType::SEMICOLON: return "SEMICOLON";
        case quill::TokenType::LPAREN: return "LPAREN";
        case quill::TokenType::RPAREN: return "RPAREN";
        case quill::TokenType::COMMA: return "COMMA";
        case quill::TokenType::STAR: return "STAR";
        case quill::TokenType::END_OF_FILE: return "EOF";
        case quill::TokenType::ILLEGAL: return "ILLEGAL";
        default: return "UNKNOWN";
    }
}

int main() {
    std::cout << "--- QuillDB Lexer Test ---" << std::endl;

    // A real SQL string to test our multi-character logic
    std::string sql = "SELECT id, name FROM users WHERE id = 42;"; 
    
    quill::Lexer lexer(sql);
    std::vector<quill::Token> tokens = lexer.tokenize();

    for (const auto& token : tokens) {
        std::cout << "Token: [" << tokenTypeToString(token.type) 
                  << "] \tLiteral: '" << token.literal << "'" << std::endl;
    }

    return 0;
}