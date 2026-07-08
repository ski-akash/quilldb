#pragma once

#include "planner/LogicalPlan.h"
#include "storage/Storage.h"
#include <memory>
#include <vector>

namespace quill {

// The base class for all physical operators
class Executor {
public:
    virtual ~Executor() = default;
    
    virtual void init() = 0;
    
    // NEW: Returns true if a row was fetched, false if no more data.
    // The fetched data is placed inside the 'out_tuple' parameter.
    virtual bool next(Tuple& out_tuple) = 0;
};

class SeqScanExecutor : public Executor {
public:
    explicit SeqScanExecutor(std::shared_ptr<Table> table);
    void init() override;
    bool next(Tuple& out_tuple) override;

private:
    std::shared_ptr<Table> table_;
    size_t current_row_ = 0;
};

class FilterExecutor : public Executor {
public:
    FilterExecutor(std::unique_ptr<Executor> child, 
                   std::shared_ptr<Expression> predicate, 
                   std::shared_ptr<Table> table_schema);
    
    void init() override;
    bool next(Tuple& out_tuple) override;

private:
    std::unique_ptr<Executor> child_;
    std::shared_ptr<Expression> predicate_;
    std::shared_ptr<Table> table_schema_; 
};

class ProjectExecutor : public Executor {
public:
    ProjectExecutor(std::unique_ptr<Executor> child, 
                    std::vector<std::shared_ptr<Expression>> columns,
                    std::shared_ptr<Table> table_schema);
    
    void init() override;
    bool next(Tuple& out_tuple) override;

private:
    std::unique_ptr<Executor> child_;
    std::vector<std::shared_ptr<Expression>> columns_;
    std::shared_ptr<Table> table_schema_;
};

} // namespace quill