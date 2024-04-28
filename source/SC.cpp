#include <SC.h>
#include <ast.h>
#include <iostream>
#include <algorithm>

SuperCombinator::SuperCombinator(LambdaNode abs): Node(abs), body(abs.body) {
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

// optimization to avoid cases
// SuperCombinator this(x, y, z) := Z(x, y, z)
// eta_conversion will return Z combinator in this case
// otherwise this
std::shared_ptr<SuperCombinator> SuperCombinator::eta_conversion()  {
    std::shared_ptr<Node> begin = std::dynamic_pointer_cast<AppNode>(body);
    int i = some_vars.size() - 1;

    while (begin != nullptr && i >= 0)
    {
        auto app = std::dynamic_pointer_cast<AppNode>(begin);
        if (app == nullptr) return shared_from_this();
        auto var = std::dynamic_pointer_cast<VarNode>(app->GetArg());
        if (var == nullptr) return shared_from_this();
        if (var->GetName() != some_vars[i]->GetName()) return shared_from_this();

        begin = app->GetFunc();
        --i;   
    }

    auto func = std::dynamic_pointer_cast<SuperCombinator>(begin);
    if (func == nullptr) return shared_from_this();
    return func;
}

std::shared_ptr<Node> SuperCombinator::Substitution(std::shared_ptr<LambdaNode> abs) {
    auto comb = std::make_shared<SuperCombinator>(*abs);
    comb = comb->eta_conversion();
    std::shared_ptr< Node> res = comb;

    int n = int(comb->some_vars.size()) - 1;
    for (int i = 0; i < n - 1; ++i) {
        res = std::make_shared<AppNode>(res, comb->some_vars[i]);
    }

    return res;
}

SC_container GetFromAst(Node* root) {
    
}