#pragma once

#include "planner/LogicalPlan.h"
#include "storage/Storage.h"
#include <memory>
#include <optional>
#include <vector>

namespace quill {

// The base class for all physical operators
class Executor {
public:
    virtual ~Executor() = default;
    
    // Resets the executor to begin pulling rows from the start
    virtual void init() = 0;
    
    // Fetches the next row. Returns std::nullopt when there is no more data.
    virtual std::optional<Tuple> next() = 0;
};

// 1. SCAN: Reads rows sequentially from our in-memory table
class SeqScanExecutor : public Executor {
public:
    explicit SeqScanExecutor(std::shared_ptr<Table> table);
    void init() override;
    std::optional<Tuple> next() override;

private:
    std::shared_ptr<Table> table_;
    size_t current_row_ = 0;
};

// 2. FILTER: Evaluates the WHERE clause and drops failing rows
class FilterExecutor : public Executor {
public:
    FilterExecutor(std::unique_ptr<Executor> child, 
                   std::shared_ptr<Expression> predicate, 
                   std::shared_ptr<Table> table_schema);
    
    void init() override;
    std::optional<Tuple> next() override;

private:
    std::unique_ptr<Executor> child_;
    std::shared_ptr<Expression> predicate_;
    std::shared_ptr<Table> table_schema_; // Needed to map column names to array indices
};

// 3. PROJECT: Slices the row to only return requested columns
class ProjectExecutor : public Executor {
public:
    ProjectExecutor(std::unique_ptr<Executor> child, 
                    std::vector<std::shared_ptr<Expression>> columns,
                    std::shared_ptr<Table> table_schema);
    
    void init() override;
    std::optional<Tuple> next() override;

private:
    std::unique_ptr<Executor> child_;
    std::vector<std::shared_ptr<Expression>> columns_;
    std::shared_ptr<Table> table_schema_;
};

} // namespace quill