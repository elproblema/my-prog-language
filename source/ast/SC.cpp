#include <algorithm>
#include <cassert>
#include <iterator>
#include <memory>
#include <vector>

#include <ast/SC.h>
#include <ast/ast.h>
#include <exceptions.h>

#define SC SuperCombinator

SC_container all_functions;

SC::SC(LambdaNode abs): Node(abs), body(abs.body), func_name(CreateName()) {
    std::vector<std::weak_ptr<VarNode>> all_vars;
    std::copy(abs.free_var.begin(), abs.free_var.end(), std::back_inserter(all_vars));
    std::copy(abs.bonded.begin(), abs.bonded.end(), std::back_inserter(all_vars));

    std::sort(all_vars.begin(), all_vars.end(), 
    [] (std::weak_ptr<VarNode> lhs, std::weak_ptr<VarNode> rhs) {
        return 
        lhs.lock()->GetHead().lock()->GetHeight() 
        > 
        rhs.lock()->GetHead().lock()->GetHeight();
    });

    auto it = std::unique(all_vars.begin(), all_vars.end(), 
    [] (std::weak_ptr<VarNode> lhs, std::weak_ptr<VarNode> rhs) {
        return 
        lhs.lock()->GetHead().lock()->GetHeight() 
        ==
        rhs.lock()->GetHead().lock()->GetHeight();
    });

    for (auto i = all_vars.begin(); i != it; ++i) {
        some_vars.push_back((*i));
    }
}

std::string Node::CreateName() {
    return std::to_string(loc.GetLine()) + ":" + std::to_string(loc.GetColumn());
}

const std::string& SC::GetFuncName() const {
    return func_name;
}

bool BIFNode::IsFunc() const {
    return true;
}

bool SC::IsFunc() const {
    return true;
}

// optimization to avoid cases
// SuperCombinator this(x, y, z) := Z(x, y, z)
// eta_conversion will return Z combinator in this case
// otherwise this
std::shared_ptr<Node> SC::EtaConversion()  {
    std::shared_ptr<Node> begin = body;
    int i = int(some_vars.size()) - 1;

    while (begin != nullptr && i >= 0)
    {
        auto var = begin->GetArg();
        if (var == nullptr) goto ret; // :)
        if (var->SafeGetName() != some_vars[i].lock()->GetName()) goto ret; // :)
        begin = begin->GetFunc();
        --i;
    }

    if (!begin->IsFunc()) goto ret; // :)
    return begin;

ret:
    all_functions.push_back(shared_from_this()); 
    return shared_from_this();
}

const std::vector<std::weak_ptr<VarNode>>&
SC::GetSomeVars() const { return some_vars; }

std::shared_ptr<Node> SC::Substitution(std::shared_ptr<LambdaNode> abs) {
    std::shared_ptr<Node> comb = std::make_shared<SC>(*abs);
    std::shared_ptr<LambdaNode> ind_node = std::shared_ptr<LambdaNode>(new LambdaNode());
    std::shared_ptr<Node> res = ind_node;

    int n = int(comb->GetSomeVars().size());
    for (int i = 0; i < n - 1; ++i) {
        res = std::make_shared<AppNode>(res, comb->GetSomeVars()[i].lock());
    }
    ind_node->body = comb->EtaConversion();
    
    return res;
}

void Node::Substitute() {
}

void LambdaNode::Substitute() {
    if (ind_tag) body->Substitute();
    body = SC::Substitution(shared_from_this());
    SetIndirected();
}

void RebuildAst(std::shared_ptr<Node> vertex, bool prog, std::string name) {
    if (prog && vertex->GetFreeVar().size()) throw FreeVariablesException();
    auto children = vertex->GetChildren();
    for (auto i : children) {
        RebuildAst(i, false);
    }
    vertex->Substitute();
    if (prog) {
        auto ptr = std::make_shared<SC>(vertex, name);
        all_functions.push_back(ptr);
    }
}