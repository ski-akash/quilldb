#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Logical Planner Test (Aggregates) ===\n\n";

    std::string sql = "SELECT user_id, SUM(amount) FROM orders GROUP BY user_id;"; 
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