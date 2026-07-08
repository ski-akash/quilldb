#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace quill {

// A single row of data
using Tuple = std::vector<std::string>;

// A naive in-memory table
class Table {
public:
    std::string name;
    std::vector<std::string> columns;
    std::vector<Tuple> rows;

    Table(std::string n, std::vector<std::string> cols)
        : name(std::move(n)), columns(std::move(cols)) {}

    void insert(Tuple row) {
        rows.push_back(std::move(row));
    }

    // Helper to find which index a column is at (e.g., "name" -> 1)
    int getColumnIndex(const std::string& colName) const {
        for (size_t i = 0; i < columns.size(); ++i) {
            if (columns[i] == colName) return i;
        }
        return -1;
    }
};

} // namespace quill