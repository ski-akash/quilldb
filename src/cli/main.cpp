#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Logical Planner Test (Joins) ===\n\n";

    std::string sql = "SELECT name, amount FROM users JOIN orders ON users.id = orders.user_id WHERE id = 42;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    // 1. Front-End
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();

    // 2. Middle-End
    quill::Planner planner;
    if (!statements.empty()) {
        std::shared_ptr<quill::PlanNode> logicalPlan = planner.createPlan(statements[0]);
        
        std::cout << "Generated Logical Plan:\n";
        std::cout << logicalPlan->toString() << std::endl;
    }

    return 0;
}