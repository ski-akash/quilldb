#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "executor/Executor.h"
#include "storage/Storage.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Vectorized Join Engine ===\n\n";

    // 1. STORAGE: Create users table
    auto users_table = std::make_shared<quill::Table>("users", std::vector<std::string>{"id", "name"});
    users_table->insert({"1", "Alice"});
    users_table->insert({"42", "Bob"});
    users_table->insert({"100", "Charlie"});

    // 2. STORAGE: Create orders table
    auto orders_table = std::make_shared<quill::Table>("orders", std::vector<std::string>{"order_id", "user_id", "amount"});
    orders_table->insert({"A01", "1", "$50"});
    orders_table->insert({"A02", "42", "$120"});  // Bob's first order
    orders_table->insert({"A03", "42", "$999"});  // Bob's second order!

    // 3. FRONT-END: Lex & Parse
    std::string sql = "SELECT name, amount FROM users JOIN orders ON users.id = orders.user_id WHERE id = 42;"; 
    std::cout << "Executing Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();
    auto selectStmt = std::dynamic_pointer_cast<quill::SelectStatement>(statements[0]);

    // 4. BACK-END: Wire up the physical pipeline
    auto left_scan = std::make_unique<quill::SeqScanExecutor>(users_table);
    auto right_scan = std::make_unique<quill::SeqScanExecutor>(orders_table);
    
    // Notice how the Join takes BOTH scans!
    auto join = std::make_unique<quill::NestedLoopJoinExecutor>(
        std::move(left_scan), std::move(right_scan), 
        selectStmt->joins[0]->condition, 
        users_table, orders_table
    );
    
    // Wrap the Join in a Filter
    // Note: To keep the C++ simple, our basic filter uses the users schema to find "id = 42"
    auto filter = std::make_unique<quill::FilterExecutor>(std::move(join), selectStmt->whereClause, users_table);

    // 5. EXECUTE!
    std::cout << "RESULTS:\n";
    std::cout << "--------------------------\n";
    
    filter->init();
    
    quill::Chunk chunk;
    while (filter->next(chunk)) {
        for (size_t row_idx = 0; row_idx < chunk.size; ++row_idx) {
            // Print Name (Index 1 from Left Table) and Amount (Index 2 from Right Table)
            std::cout << chunk.columns[1][row_idx] << "\t| " << chunk.columns[2 + 2][row_idx] << "\n";
        }
    }
    
    std::cout << "--------------------------\n";

    return 0;
}