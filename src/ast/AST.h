#pragma once

#include <string>
#include <vector>
#include <memory>

namespace quill {

// ---------------------------------------------------------
// BASE INTERFACES
// ---------------------------------------------------------

// The root of all AST nodes
class Node {
public:
    virtual ~Node() = default;
    
    // Every node must be able to print itself for debugging
    virtual std::string toString() const = 0; 
};

// Base class for things that produce values (columns, numbers, math)
class Expression : public Node {
public:
    virtual ~Expression() = default;
};

// Base class for full commands (SELECT, INSERT, CREATE)
class Statement : public Node {
public:
    virtual ~Statement() = default;
};

// ---------------------------------------------------------
// SPECIFIC IMPLEMENTATIONS
// ---------------------------------------------------------

// Represents a column name or a table name
class Identifier : public Expression {
public:
    std::string value;

    explicit Identifier(std::string val) : value(std::move(val)) {}

    std::string toString() const override {
        return value;
    }
};

// Represents a full SELECT query
class SelectStatement : public Statement {
public:
    // A query can have multiple columns: SELECT id, name, age ...
    std::vector<std::shared_ptr<Expression>> columns;
    
    // The table we are querying from
    std::string tableName;

    std::string toString() const override {
        std::string result = "SELECT [ ";
        for (size_t i = 0; i < columns.size(); ++i) {
            result += columns[i]->toString();
            if (i < columns.size() - 1) result += ", ";
        }
        result += " ] FROM " + tableName;
        return result;
    }
};

} // namespace quill