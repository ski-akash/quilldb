#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "--- QuillDB Logical Planner Test ---" << std::endl;

    std::string sql = "SELECT id, name FROM users WHERE id = 42;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    // 1. Lexical & Syntax Analysis (Front-End)
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    std::vector<std::shared_ptr<quill::Statement>> statements = parser.parse();

    // 2. Logical Planning (Middle-End)
    quill::Planner planner;
    
    // We assume the first statement is our SELECT query for this test
    if (!statements.empty()) {
        std::shared_ptr<quill::PlanNode> logicalPlan = planner.createPlan(statements[0]);
        
        std::cout << "Generated Logical Plan (Relational Algebra):\n";
        std::cout << logicalPlan->toString() << std::endl;
    }

    return 0;
}