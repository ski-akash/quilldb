#pragma once

#include "planner/LogicalPlan.h"
#include <memory>

namespace quill {

class Optimizer {
public:
    // Entry point: takes a raw logical plan and returns an optimized one
    std::shared_ptr<PlanNode> optimize(std::shared_ptr<PlanNode> plan);

private:
    // Rule 1: Push filters as close to the data source (Scan) as possible
    std::shared_ptr<PlanNode> applyPredicatePushdown(std::shared_ptr<PlanNode> plan);
};

} // namespace quill