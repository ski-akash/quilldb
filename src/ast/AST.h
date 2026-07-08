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

// NEW: Represents a numeric literal (e.g., 42)
class NumberLiteral : public Expression {
public:
    std::string value;
    explicit NumberLiteral(std::string val) : value(std::move(val)) {}
    std::string toString() const override { return value; }
};

// NEW: Represents operations like 'id = 42' or 'age > 18'
class BinaryExpression : public Expression {
public:
    std::shared_ptr<Expression> left;
    std::string op; // The operator (e.g., "=")
    std::shared_ptr<Expression> right;

    BinaryExpression(std::shared_ptr<Expression> l, std::string o, std::shared_ptr<Expression> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    std::string toString() const override {
        return "(" + left->toString() + " " + op + " " + right->toString() + ")";
    }
};

// NEW: Represents a JOIN ... ON ... clause
class JoinClause : public Node {
public:
    std::string tableName;
    std::shared_ptr<Expression> condition; // e.g., users.id = orders.user_id

    JoinClause(std::string table, std::shared_ptr<Expression> cond)
        : tableName(std::move(table)), condition(std::move(cond)) {}

    std::string toString() const override {
        return "JOIN " + tableName + " ON " + condition->toString();
    }
};

// Represents a full SELECT query
class SelectStatement : public Statement {
public:
    std::vector<std::shared_ptr<Expression>> columns;
    std::string tableName;
    
    // NEW: A query can have multiple joins
    std::vector<std::shared_ptr<JoinClause>> joins;
    
    std::shared_ptr<Expression> whereClause = nullptr;

    std::string toString() const override {
        std::string result = "SELECT [ ";
        for (size_t i = 0; i < columns.size(); ++i) {
            result += columns[i]->toString();
            if (i < columns.size() - 1) result += ", ";
        }
        result += " ] FROM " + tableName;
        
        // NEW: Print joins if they exist
        for (const auto& join : joins) {
            result += " " + join->toString();
        }
        
        if (whereClause != nullptr) {
            result += " WHERE " + whereClause->toString();
        }
        
        return result;
    }
}; 

}