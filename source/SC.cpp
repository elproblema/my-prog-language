#include <SC.h>
#include <ast.h>
#include <algorithm>
#include <cassert>

#define SC SuperCombinator

SC::SC(LambdaNode abs): Node(abs), body(abs.body), name(create_name()) {
    std::vector<std::weak_ptr<VarNode>> all_vars;
    std::copy(abs.free_var.begin(), abs.free_var.end(), std::back_inserter(all_vars));
    std::copy(abs.bonded.begin(), abs.bonded.end(), std::back_inserter(all_vars));

    std::sort(all_vars.begin(), all_vars.end(), 
    [] (std::weak_ptr<VarNode> lhs, std::weak_ptr<VarNode> rhs) {
        return lhs.lock()->GetDepth() < rhs.lock()->GetDepth();
    });

    auto it = std::unique(all_vars.begin(), all_vars.end(), 
    [] (std::weak_ptr<VarNode> lhs, std::weak_ptr<VarNode> rhs) {
        return lhs.lock()->GetDepth() == rhs.lock()->GetDepth();
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
// sorry :)
#define ret \
all_functions.push_back(shared_from_this()); \
return shared_from_this()

    std::shared_ptr<Node> begin = std::dynamic_pointer_cast<AppNode>(body);
    int i = some_vars.size() - 1;

    while (begin != nullptr && i >= 0)
    {
        auto var = begin->GetArg();
        if (var == nullptr) ret;
        if (var->GetName() != some_vars[i].lock()->GetName()) ret;

        begin = begin->GetFunc();
        --i;   
    }

    auto func = std::dynamic_pointer_cast<SuperCombinator>(begin);
    if (func == nullptr) return shared_from_this();
    return func;
}

std::shared_ptr<Node> SC::Substitution(std::shared_ptr<LambdaNode> abs) {
    auto comb = std::make_shared<SuperCombinator>(*abs);
    comb = comb->eta_conversion();
    std::shared_ptr<Node> res = comb;

    int n = int(comb->some_vars.size()) - 1;
    for (int i = 0; i < n - 1; ++i) {
        res = std::make_shared<AppNode>(res, comb->some_vars[i].lock());
    }

    return res;
}

void Node::Substitute() {}

void LambdaNode::Substitute() {
    body = SC::Substitution(shared_from_this());
}

void RebuildAst(std::shared_ptr<Node> vertex, bool prog=true, std::string name="PROG") {
    SC_container res;
    auto children = vertex->GetChildren();
    for (auto i : children) RebuildAst(i, false);
    vertex->Substitute();
    if (prog) {
        auto ptr = std::make_shared<SuperCombinator>(vertex, name);
        all_functions.push_back(ptr);
    }
}