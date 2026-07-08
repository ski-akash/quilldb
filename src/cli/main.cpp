#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "executor/Executor.h"
#include "storage/Storage.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Vectorized Aggregation Engine ===\n\n";

    // 1. STORAGE: Create orders table with multiple orders per user
    auto orders_table = std::make_shared<quill::Table>("orders", std::vector<std::string>{"order_id", "user_id", "amount"});
    orders_table->insert({"A01", "42", "$50"});
    orders_table->insert({"A02", "42", "$120"});  // Bob ordered twice! (Total: 170)
    orders_table->insert({"A03", "100", "$999"}); // Charlie's massive order (Total: 999)
    orders_table->insert({"A04", "100", "$1"});   // Charlie's tiny order (Total: 1000)
    orders_table->insert({"A05", "1", "$25"});    // Alice's order (Total: 25)

    // 2. FRONT-END: Lex & Parse
    std::string sql = "SELECT user_id, SUM(amount) FROM orders GROUP BY user_id;"; 
    std::cout << "Executing Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();
    auto selectStmt = std::dynamic_pointer_cast<quill::SelectStatement>(statements[0]);

    // 3. BACK-END: Wire up the physical pipeline
    auto scan = std::make_unique<quill::SeqScanExecutor>(orders_table);
    
    // Check if the query has aggregates
    std::vector<std::shared_ptr<quill::Expression>> aggregates;
    for (const auto& col : selectStmt->columns) {
        if (std::dynamic_pointer_cast<quill::FunctionCall>(col)) {
            aggregates.push_back(col);
        }
    }

    // Run the Aggregate pipeline
    auto aggregate = std::make_unique<quill::AggregateExecutor>(
        std::move(scan), selectStmt->groupBy, aggregates, orders_table
    );

    // 4. EXECUTE!
    std::cout << "RESULTS (User ID | Total Spend):\n";
    std::cout << "--------------------------\n";
    
    aggregate->init();
    
    quill::Chunk chunk;
    while (aggregate->next(chunk)) {
        for (size_t row_idx = 0; row_idx < chunk.size; ++row_idx) {
            std::cout << chunk.columns[0][row_idx] << "\t\t | " << chunk.columns[1][row_idx] << "\n";
        }
    }
    
    std::cout << "--------------------------\n";

    return 0;
}