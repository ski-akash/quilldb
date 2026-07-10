#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "optimizer/Optimizer.h"
#include <iostream>

int main() {
    std::cout << "=== QuillDB EXPLAIN Command ===\n\n";

    // Notice the EXPLAIN keyword at the start!
    std::string sql = "EXPLAIN SELECT name FROM users WHERE id = 42;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();

    if (statements.empty()) return 1;

    // Check if it's an EXPLAIN statement
    auto explainStmt = std::dynamic_pointer_cast<quill::ExplainStatement>(statements[0]);
    std::shared_ptr<quill::Statement> targetStmt = explainStmt ? explainStmt->statement : statements[0];

    // 1. Plan the query
    quill::Planner planner;
    auto logicalPlan = planner.createPlan(targetStmt);

    // 2. Optimize the plan (Predicate Pushdown)
    quill::Optimizer optimizer;
    
    // Artificially create a sub-optimal plan for our test (Filter -> Project -> Scan)
    auto subOptimalPlan = std::make_shared<quill::FilterNode>(
        logicalPlan, 
        std::dynamic_pointer_cast<quill::SelectStatement>(targetStmt)->whereClause
    );
    std::dynamic_pointer_cast<quill::ProjectNode>(logicalPlan)->child = 
        std::dynamic_pointer_cast<quill::FilterNode>(std::dynamic_pointer_cast<quill::ProjectNode>(logicalPlan)->child)->child;

    auto optimizedPlan = optimizer.optimize(subOptimalPlan);

    // 3. If EXPLAIN, print the optimized plan and halt execution!
    if (explainStmt) {
        std::cout << "--- OPTIMIZED EXECUTION PLAN ---\n";
        std::cout << optimizedPlan->toString() << "\n\n";
        std::cout << "(Execution skipped due to EXPLAIN keyword)\n";
        return 0; 
    }

    // (If not EXPLAIN, physical execution would happen here...)

    return 0;
}