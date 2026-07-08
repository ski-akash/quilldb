#include "planner/Planner.h"

namespace quill {

std::shared_ptr<PlanNode> Planner::createPlan(const std::shared_ptr<Statement>& stmt) {
    // We use dynamic_cast to figure out what type of Statement the parser gave us
    auto selectStmt = std::dynamic_pointer_cast<SelectStatement>(stmt);
    
    if (selectStmt) {
        return planSelect(selectStmt);
    }
    
    throw std::runtime_error("Unsupported statement type in planner.");
}

std::shared_ptr<PlanNode> Planner::planSelect(const std::shared_ptr<SelectStatement>& stmt) {
    // 1. SCAN: The bottom of our pipeline starts with the main table
    std::shared_ptr<PlanNode> root = std::make_shared<SeqScanNode>(stmt->tableName);

    // 2. JOIN: If there are joins, wrap the current root and a scan of the joined table
    for (const auto& join : stmt->joins) {
        auto right_scan = std::make_shared<SeqScanNode>(join->tableName);
        root = std::make_shared<NestedLoopJoinNode>(root, right_scan, join->condition);
    }

    // 3. FILTER: If there is a WHERE clause, wrap the tree in a FILTER
    if (stmt->whereClause != nullptr) {
        root = std::make_shared<FilterNode>(root, stmt->whereClause);
    }

    // 4. PROJECT: Wrap everything in a PROJECT to return only the requested columns
    root = std::make_shared<ProjectNode>(root, stmt->columns);

    return root; 
}

} // namespace quill