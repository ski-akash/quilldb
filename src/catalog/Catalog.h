#pragma once

#include <string>
#include <unordered_map>

namespace quill {

class Catalog {
public:
    // Store the number of rows for a given table
    void setTableRowCount(const std::string& tableName, size_t rowCount);
    
    // Retrieve the number of rows (defaults to a high number if unknown to force safe planning)
    size_t getTableRowCount(const std::string& tableName) const;

private:
    std::unordered_map<std::string, size_t> table_row_counts_;
};

} // namespace quill