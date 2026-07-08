#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "executor/Executor.h"
#include "storage/Storage.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Vectorized Engine ===\n\n";

    auto users_table = std::make_shared<quill::Table>("users", std::vector<std::string>{"id", "name", "email"});
    // Insert some test data (our Table class still lets us insert rows, but shreds them into columns internally)
    users_table->insert({"1", "Alice", "alice@example.com"});
    users_table->insert({"42", "Bob", "bob@example.com"});
    users_table->insert({"100", "Charlie", "charlie@example.com"});

    std::string sql = "SELECT name, email FROM users WHERE id = 42;"; 
    std::cout << "Executing Query: " << sql << "\n\n";
    
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();
    auto selectStmt = std::dynamic_pointer_cast<quill::SelectStatement>(statements[0]);

    auto scan = std::make_unique<quill::SeqScanExecutor>(users_table);
    auto filter = std::make_unique<quill::FilterExecutor>(std::move(scan), selectStmt->whereClause, users_table);
    auto project = std::make_unique<quill::ProjectExecutor>(std::move(filter), selectStmt->columns, users_table);

    std::cout << "RESULTS:\n";
    std::cout << "--------------------------\n";
    
    project->init();
    
    quill::Chunk chunk;
    // Iterate over batches/chunks
    while (project->next(chunk)) {
        // Iterate over the rows inside this specific chunk
        for (size_t row_idx = 0; row_idx < chunk.size; ++row_idx) {
            // Iterate over the columns for this row
            for (size_t col_idx = 0; col_idx < chunk.columns.size(); ++col_idx) {
                std::cout << chunk.columns[col_idx][row_idx] << "\t| ";
            }
            std::cout << "\n";
        }
    }
    
    std::cout << "--------------------------\n";

    return 0;
}