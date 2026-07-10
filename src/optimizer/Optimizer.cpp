#include "optimizer/Optimizer.h"

namespace quill {

std::shared_ptr<PlanNode> Optimizer::optimize(std::shared_ptr<PlanNode> plan) {
    // We can chain multiple optimization passes here in the future
    plan = applyPredicatePushdown(plan);
    return plan;
}

std::shared_ptr<PlanNode> Optimizer::applyPredicatePushdown(std::shared_ptr<PlanNode> plan) {
    if (!plan) return nullptr;

    // 1. If it's a Project, recursively optimize its child
    if (auto projectNode = std::dynamic_pointer_cast<ProjectNode>(plan)) {
        projectNode->child = applyPredicatePushdown(projectNode->child);
        return projectNode;
    } 
    // 2. If it's a Filter, check if we can push it down!
    else if (auto filterNode = std::dynamic_pointer_cast<FilterNode>(plan)) {
        
        // First, recursively optimize the child
        filterNode->child = applyPredicatePushdown(filterNode->child);

        // THE PUSHDOWN RULE:
        // If this Filter is sitting on top of a Project, swap them!
        // (Filter -> Project -> Scan) becomes (Project -> Filter -> Scan)
        if (auto childProject = std::dynamic_pointer_cast<ProjectNode>(filterNode->child)) {
            
            // Rewire the tree
            filterNode->child = childProject->child; // Filter now points to Scan
            childProject->child = filterNode;        // Project now points to Filter
            
            return childProject; // The Project is now the new top of this sub-tree
        }
        
        return filterNode;
    }
    // 3. Traverse through Joins
    else if (auto joinNode = std::dynamic_pointer_cast<NestedLoopJoinNode>(plan)) {
        joinNode->left_child = applyPredicatePushdown(joinNode->left_child);
        joinNode->right_child = applyPredicatePushdown(joinNode->right_child);
        return joinNode;
    }
    // 4. Traverse through Aggregates
    else if (auto aggNode = std::dynamic_pointer_cast<AggregateNode>(plan)) {
        aggNode->child = applyPredicatePushdown(aggNode->child);
        return aggNode;
    }

    // Base case (like a Scan Node)
    return plan;
}

} // namespace quill