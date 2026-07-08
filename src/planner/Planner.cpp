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
    // 1. SCAN: The bottom of our pipeline is always reading the table
    std::shared_ptr<PlanNode> root = std::make_shared<SeqScanNode>(stmt->tableName);

    // 2. FILTER: If there is a WHERE clause, wrap the SCAN in a FILTER
    if (stmt->whereClause != nullptr) {
        root = std::make_shared<FilterNode>(root, stmt->whereClause);
    }

    // 3. PROJECT: Wrap everything in a PROJECT to return only the requested columns
    root = std::make_shared<ProjectNode>(root, stmt->columns);

    return root; // Return the top of the tree
}

} // namespace quill