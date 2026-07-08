#pragma once

#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace quill {

// A single row of data (still used for inserting data easily)
using Tuple = std::vector<std::string>;

// A batch of columnar data (used by our vectorized executors)
struct Chunk {
    // Each element in the outer vector is a column.
    // The inner vector contains the data for that column.
    std::vector<std::vector<std::string>> columns;
    size_t size = 0; // Number of rows in this chunk
};

// A Columnar in-memory table
class Table {
public:
    std::string name;
    std::vector<std::string> column_names;
    
    // NEW: Data is stored as columns, not rows!
    // column_data_[0] contains all values for the first column.
    std::vector<std::vector<std::string>> column_data_;
    size_t num_rows_ = 0;

    Table(std::string n, std::vector<std::string> cols)
        : name(std::move(n)), column_names(std::move(cols)) {
        // Initialize an empty vector for each column
        column_data_.resize(column_names.size());
    }

    // We still accept a row for easy insertion, but we shred it into columns internally
    void insert(const Tuple& row) {
        if (row.size() != column_names.size()) {
            throw std::runtime_error("Insert failed: Row size does not match column count.");
        }
        for (size_t i = 0; i < row.size(); ++i) {
            column_data_[i].push_back(row[i]);
        }
        num_rows_++;
    }

    int getColumnIndex(const std::string& colName) const {
        for (size_t i = 0; i < column_names.size(); ++i) {
            if (column_names[i] == colName) return i;
        }
        return -1;
    }
};

} // namespace quill