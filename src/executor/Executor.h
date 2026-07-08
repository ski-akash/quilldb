#pragma once

#include "planner/LogicalPlan.h"
#include "storage/Storage.h"
#include <memory>
#include <vector>

namespace quill {

// How many rows we process in a single chunk
constexpr size_t BATCH_SIZE = 100; 

class Executor {
public:
    virtual ~Executor() = default;
    
    virtual void init() = 0;
    
    // NEW: Returns true if a CHUNK of rows was fetched, false if no more data.
    virtual bool next(Chunk& out_chunk) = 0;
};

class SeqScanExecutor : public Executor {
public:
    explicit SeqScanExecutor(std::shared_ptr<Table> table);
    void init() override;
    bool next(Chunk& out_chunk) override;

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
    bool next(Chunk& out_chunk) override;

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
    bool next(Chunk& out_chunk) override;

private:
    std::unique_ptr<Executor> child_;
    std::vector<std::shared_ptr<Expression>> columns_;
    std::shared_ptr<Table> table_schema_;
};

// 4. NESTED LOOP JOIN (Vectorized)
class NestedLoopJoinExecutor : public Executor {
public:
    NestedLoopJoinExecutor(std::unique_ptr<Executor> left, 
                           std::unique_ptr<Executor> right, 
                           std::shared_ptr<Expression> predicate,
                           std::shared_ptr<Table> left_schema,
                           std::shared_ptr<Table> right_schema);
    
    void init() override;
    bool next(Chunk& out_chunk) override;

private:
    std::unique_ptr<Executor> left_child_;
    std::unique_ptr<Executor> right_child_;
    std::shared_ptr<Expression> predicate_;
    
    std::shared_ptr<Table> left_schema_;
    std::shared_ptr<Table> right_schema_;
    
    // We "materialize" the right table's chunks into memory during init()
    std::vector<Chunk> right_materialized_; 
};

} // namespace quill