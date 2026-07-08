#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Parser Test (Aggregates) ===\n\n";

    // A full analytical query with Aggregates and Grouping
    std::string sql = "SELECT user_id, SUM(amount) FROM orders GROUP BY user_id;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();

    for (const auto& stmt : statements) {
        std::cout << "Generated AST Node:\n";
        std::cout << stmt->toString() << std::endl;
    }

    return 0;
}