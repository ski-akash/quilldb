#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set> // NEW

namespace quill {

class Catalog {
public:
    void setTableRowCount(const std::string& tableName, size_t rowCount);
    size_t getTableRowCount(const std::string& tableName) const;

    // NEW: Track which columns have an index
    void addIndex(const std::string& tableName, const std::string& columnName);
    bool hasIndex(const std::string& tableName, const std::string& columnName) const;

private:
    std::unordered_map<std::string, size_t> table_row_counts_;
    
    // Map of Table Name -> Set of Indexed Column Names
    std::unordered_map<std::string, std::unordered_set<std::string>> table_indexes_;
};

} // namespace quill