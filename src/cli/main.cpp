#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "--- QuillDB Parser Test ---" << std::endl;

    // A real SQL string to test our AST generation
    std::string sql = "SELECT id, name, email FROM users;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    // 1. Lexical Analysis
    quill::Lexer lexer(sql);
    
    // 2. Parsing (Syntax Analysis)
    // We use std::move because the Parser takes ownership of the Lexer
    quill::Parser parser(std::move(lexer));
    
    // Generate the AST
    std::vector<std::shared_ptr<quill::Statement>> statements = parser.parse();

    // 3. Output the AST
    for (const auto& stmt : statements) {
        std::cout << "Generated AST Node:\n";
        std::cout << stmt->toString() << std::endl;
    }

    return 0;
}