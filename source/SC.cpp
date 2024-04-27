#include <SC.h>
#include <ast.h>
#include <iostream>
#include <algorithm>

SuperCombinator::SuperCombinator(LambdaNode abs): Node(abs) {
    std::vector<std::shared_ptr<VarNode>> all_vars;
    std::copy(abs.free_var.begin(), abs.free_var.end(), std::back_inserter(all_vars));
    std::copy(abs.bonded.begin(), abs.bonded.end(), std::back_inserter(all_vars));
    std::sort(all_vars.begin(), all_vars.end(), 
    [] (std::shared_ptr<VarNode> lhs, std::shared_ptr<VarNode> rhs) {
        return lhs->GetDepth() < rhs->GetDepth();
    });
    auto it = std::unique(all_vars.begin(), all_vars.end(), 
    [] (std::shared_ptr<VarNode> lhs, std::shared_ptr<VarNode> rhs) {
        return lhs->GetDepth() == rhs->GetDepth();
    });
    for (auto i = all_vars.begin(); i != it; ++it) {
        some_vars.push_back((*i));
    }
}

std::string SuperCombinator::create_name() {
    return std::to_string(loc.GetLine()) + ":" + std::to_string(loc.GetColumn());
}

std::shared_ptr<Node> SuperCombinator::Substitution(std::shared_ptr<LambdaNode> abs) {
    auto comb = std::make_shared<SuperCombinator>(*abs);
    std::shared_ptr<Node> res = comb;

    int n = int(comb->var_names.size()) - 1;
    for (int i = 0; i < n - 1; ++i) {
        res = std::make_shared<AppNode>(res, comb->some_vars[i]);
    }

    return res;
}
