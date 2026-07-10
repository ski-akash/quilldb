#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "optimizer/Optimizer.h"
#include "catalog/Catalog.h"
#include <iostream>

int main() {
    std::cout << "=== QuillDB CBO (Cost-Based Optimizer) Test ===\n\n";

    // 1. Initialize Catalog with Statistics
    auto catalog = std::make_shared<quill::Catalog>();
    catalog->setTableRowCount("users", 100);
    catalog->setTableRowCount("orders", 50000); // Massive table!

    // 2. Front-End: Lex & Parse
    std::string sql = "EXPLAIN SELECT name, amount FROM users JOIN orders ON users.id = orders.user_id;"; 
    std::cout << "Raw Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();

    auto explainStmt = std::dynamic_pointer_cast<quill::ExplainStatement>(statements[0]);
    std::shared_ptr<quill::Statement> targetStmt = explainStmt ? explainStmt->statement : statements[0];

    // 3. Planner
    quill::Planner planner;
    auto logicalPlan = planner.createPlan(targetStmt);

    // 4. Optimizer (Now injected with the Catalog!)
    quill::Optimizer optimizer(catalog);
    auto optimizedPlan = optimizer.optimize(logicalPlan);

    // 5. Output
    if (explainStmt) {
        std::cout << "--- CBO EXPLAIN PLAN ---\n";
        std::cout << optimizedPlan->toString() << "\n\n";
    }

    return 0;
}