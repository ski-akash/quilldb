#pragma once

#include "ast/AST.h"
#include <memory>
#include <string>
#include <vector>

namespace quill {

// Base class for all relational algebra operators
class PlanNode {
public:
    virtual ~PlanNode() = default;
    
    // For debugging and the EXPLAIN command later
    virtual std::string toString() const = 0;
};

// 1. SCAN: Reads raw data from a table
class SeqScanNode : public PlanNode {
public:
    std::string tableName;

    explicit SeqScanNode(std::string name) : tableName(std::move(name)) {}

    std::string toString() const override {
        return "SeqScan(table: " + tableName + ")";
    }
};

// 2. FILTER: Applies a WHERE clause
class FilterNode : public PlanNode {
public:
    std::shared_ptr<PlanNode> child;       // The data source (e.g., a SeqScan)
    std::shared_ptr<Expression> predicate; // The math equation (e.g., id = 42)

    FilterNode(std::shared_ptr<PlanNode> childNode, std::shared_ptr<Expression> pred)
        : child(std::move(childNode)), predicate(std::move(pred)) {}

    std::string toString() const override {
        return "Filter(predicate: " + predicate->toString() + ")\n  -> " + child->toString();
    }
};

// 3. PROJECT: Selects specific columns
class ProjectNode : public PlanNode {
public:
    std::shared_ptr<PlanNode> child;                  // The data source (e.g., a Filter or Scan)
    std::vector<std::shared_ptr<Expression>> columns; // The columns to keep

    ProjectNode(std::shared_ptr<PlanNode> childNode, std::vector<std::shared_ptr<Expression>> cols)
        : child(std::move(childNode)), columns(std::move(cols)) {}

    std::string toString() const override {
        std::string cols = "";
        for (size_t i = 0; i < columns.size(); ++i) {
            cols += columns[i]->toString();
            if (i < columns.size() - 1) cols += ", ";
        }
        return "Project(columns: [" + cols + "])\n  -> " + child->toString();
    }
};

} // namespace quill