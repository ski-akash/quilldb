#include "executor/Executor.h"

namespace quill {

// ---------------------------------------------------------
// 1. SEQUENCE SCAN
// ---------------------------------------------------------
SeqScanExecutor::SeqScanExecutor(std::shared_ptr<Table> table)
    : table_(std::move(table)) {}

void SeqScanExecutor::init() {
    current_row_ = 0; 
}

bool SeqScanExecutor::next(Tuple& out_tuple) {
    if (current_row_ < table_->rows.size()) {
        out_tuple = table_->rows[current_row_++];
        return true;
    }
    return false; // No more rows
}

// ---------------------------------------------------------
// 2. FILTER
// ---------------------------------------------------------
FilterExecutor::FilterExecutor(std::unique_ptr<Executor> child, 
                               std::shared_ptr<Expression> predicate, 
                               std::shared_ptr<Table> table_schema)
    : child_(std::move(child)), predicate_(std::move(predicate)), table_schema_(std::move(table_schema)) {}

void FilterExecutor::init() {
    child_->init();
}

bool FilterExecutor::next(Tuple& out_tuple) {
    // Keep pulling rows from the child until we find one that passes
    while (child_->next(out_tuple)) {
        if (!predicate_) return true; // No WHERE clause, let it pass

        auto binary_expr = std::dynamic_pointer_cast<BinaryExpression>(predicate_);
        if (binary_expr && binary_expr->op == "=") {
            auto left_ident = std::dynamic_pointer_cast<Identifier>(binary_expr->left);
            auto right_literal = std::dynamic_pointer_cast<NumberLiteral>(binary_expr->right);

            if (left_ident && right_literal) {
                int col_idx = table_schema_->getColumnIndex(left_ident->value);
                
                // If it matches, the data is already inside 'out_tuple', so just return true
                if (col_idx != -1 && out_tuple[col_idx] == right_literal->value) {
                    return true;
                }
            }
        }
    }
    return false; 
}

// ---------------------------------------------------------
// 3. PROJECT
// ---------------------------------------------------------
ProjectExecutor::ProjectExecutor(std::unique_ptr<Executor> child, 
                                 std::vector<std::shared_ptr<Expression>> columns,
                                 std::shared_ptr<Table> table_schema)
    : child_(std::move(child)), columns_(std::move(columns)), table_schema_(std::move(table_schema)) {}

void ProjectExecutor::init() {
    child_->init();
}

bool ProjectExecutor::next(Tuple& out_tuple) {
    Tuple temp_tuple;
    
    // Pull a row from the filter into our temporary tuple
    if (child_->next(temp_tuple)) {
        out_tuple.clear(); // Empty out the returning tuple
        
        // Build a new row containing ONLY the requested columns
        for (const auto& expr : columns_) {
            auto ident = std::dynamic_pointer_cast<Identifier>(expr);
            if (ident) {
                int col_idx = table_schema_->getColumnIndex(ident->value);
                if (col_idx != -1) {
                    out_tuple.push_back(temp_tuple[col_idx]);
                }
            }
        }
        return true;
    }
    return false;
}

} // namespace quill