#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "planner/Planner.h"
#include "executor/Executor.h"
#include "storage/Storage.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "=== QuillDB Execution Engine ===\n\n";

    // 1. THE STORAGE: Create a table and insert dummy data
    auto users_table = std::make_shared<quill::Table>("users", std::vector<std::string>{"id", "name", "email"});
    users_table->insert({"1", "Alice", "alice@example.com"});
    users_table->insert({"42", "Bob", "bob@example.com"});
    users_table->insert({"100", "Charlie", "charlie@example.com"});

    std::string sql = "SELECT name, email FROM users WHERE id = 42;"; 
    std::cout << "Executing Query: " << sql << "\n\n";
    
    // 2. THE FRONT-END: Lex & Parse
    quill::Lexer lexer(sql);
    quill::Parser parser(std::move(lexer));
    auto statements = parser.parse();
    auto selectStmt = std::dynamic_pointer_cast<quill::SelectStatement>(statements[0]);

    // 3. THE BACK-END: Wire up the physical pipeline
    auto scan = std::make_unique<quill::SeqScanExecutor>(users_table);
    auto filter = std::make_unique<quill::FilterExecutor>(std::move(scan), selectStmt->whereClause, users_table);
    auto project = std::make_unique<quill::ProjectExecutor>(std::move(filter), selectStmt->columns, users_table);

    // 4. EXECUTION: Pull the data using the Volcano Iterator Model
    std::cout << "RESULTS:\n";
    std::cout << "--------------------------\n";
    
    project->init();
    
    // NEW: We pass 'row' in, and the executors fill it with data!
    quill::Tuple row;
    while (project->next(row)) {
        for (const auto& col : row) {
            std::cout << col << "\t| ";
        }
        std::cout << "\n";
    }
    
    std::cout << "--------------------------\n";

    return 0;
}