#include "executor/Executor.h"
#include <algorithm>

namespace quill {

// ---------------------------------------------------------
// 1. SEQUENCE SCAN (Vectorized)
// ---------------------------------------------------------
SeqScanExecutor::SeqScanExecutor(std::shared_ptr<Table> table)
    : table_(std::move(table)) {}

void SeqScanExecutor::init() {
    current_row_ = 0; 
}

bool SeqScanExecutor::next(Chunk& out_chunk) {
    if (current_row_ >= table_->num_rows_) return false;

    // Determine how many rows to fetch (up to BATCH_SIZE)
    size_t rows_to_fetch = std::min(BATCH_SIZE, table_->num_rows_ - current_row_);
    
    out_chunk.columns.resize(table_->column_names.size());
    for (auto& col : out_chunk.columns) col.clear();
    out_chunk.size = rows_to_fetch;

    // Bulk copy data from the table into our chunk
    for (size_t col_idx = 0; col_idx < table_->column_names.size(); ++col_idx) {
        for (size_t i = 0; i < rows_to_fetch; ++i) {
            out_chunk.columns[col_idx].push_back(table_->column_data_[col_idx][current_row_ + i]);
        }
    }
    
    current_row_ += rows_to_fetch;
    return true;
}

// ---------------------------------------------------------
// 2. FILTER (Vectorized)
// ---------------------------------------------------------
FilterExecutor::FilterExecutor(std::unique_ptr<Executor> child, 
                               std::shared_ptr<Expression> predicate, 
                               std::shared_ptr<Table> table_schema)
    : child_(std::move(child)), predicate_(std::move(predicate)), table_schema_(std::move(table_schema)) {}

void FilterExecutor::init() {
    child_->init();
}

bool FilterExecutor::next(Chunk& out_chunk) {
    Chunk in_chunk;
    
    while (child_->next(in_chunk)) {
        if (!predicate_) {
            out_chunk = in_chunk; // No WHERE clause, pass the whole chunk
            return true; 
        }

        auto binary_expr = std::dynamic_pointer_cast<BinaryExpression>(predicate_);
        if (binary_expr && binary_expr->op == "=") {
            auto left_ident = std::dynamic_pointer_cast<Identifier>(binary_expr->left);
            auto right_literal = std::dynamic_pointer_cast<NumberLiteral>(binary_expr->right);

            if (left_ident && right_literal) {
                int col_idx = table_schema_->getColumnIndex(left_ident->value);
                if (col_idx != -1) {
                    out_chunk.columns.resize(in_chunk.columns.size());
                    for (auto& col : out_chunk.columns) col.clear();
                    out_chunk.size = 0;

                    // Tight CPU loop to evaluate the filter over the whole batch
                    for (size_t row_idx = 0; row_idx < in_chunk.size; ++row_idx) {
                        if (in_chunk.columns[col_idx][row_idx] == right_literal->value) {
                            // Row passes, copy it to the output chunk
                            for (size_t c = 0; c < in_chunk.columns.size(); ++c) {
                                out_chunk.columns[c].push_back(in_chunk.columns[c][row_idx]);
                            }
                            out_chunk.size++;
                        }
                    }
                    if (out_chunk.size > 0) return true; // Yield this filtered batch
                }
            }
        }
    }
    return false; 
}

// ---------------------------------------------------------
// 3. PROJECT (Vectorized)
// ---------------------------------------------------------
ProjectExecutor::ProjectExecutor(std::unique_ptr<Executor> child, 
                                 std::vector<std::shared_ptr<Expression>> columns,
                                 std::shared_ptr<Table> table_schema)
    : child_(std::move(child)), columns_(std::move(columns)), table_schema_(std::move(table_schema)) {}

void ProjectExecutor::init() {
    child_->init();
}

bool ProjectExecutor::next(Chunk& out_chunk) {
    Chunk in_chunk;
    
    if (child_->next(in_chunk)) {
        out_chunk.columns.resize(columns_.size());
        for (auto& col : out_chunk.columns) col.clear();
        out_chunk.size = in_chunk.size;
        
        // Build a new chunk containing ONLY the requested columns
        for (size_t out_c = 0; out_c < columns_.size(); ++out_c) {
            auto ident = std::dynamic_pointer_cast<Identifier>(columns_[out_c]);
            if (ident) {
                int in_c = table_schema_->getColumnIndex(ident->value);
                if (in_c != -1) {
                    // FAST PATH: Bulk copy the entire column array at once!
                    out_chunk.columns[out_c] = in_chunk.columns[in_c]; 
                }
            }
        }
        return true;
    }
    return false;
}

} // namespace quill