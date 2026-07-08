#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "--- QuillDB Parser Test (With WHERE Clause) ---" << std::endl;

    // A full query hitting all of our Phase 1 parser requirements
    std::string sql = "SELECT id, name FROM users WHERE id = 42;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    std::vector<std::shared_ptr<quill::Statement>> statements = parser.parse();

    for (const auto& stmt : statements) {
        std::cout << "Generated AST Node:\n";
        std::cout << stmt->toString() << std::endl;
    }

    return 0;
}