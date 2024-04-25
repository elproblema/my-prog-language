#include <ast.h>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <iterator>

namespace rv = std::ranges::views;


LambdaNode::LambdaNode(VarNode&& var, Node* body): VarNode(std::move(var)), body(body) {
    free_var.clear();
    depth = body->GetDepth() + 1;
    std::function<bool(VarNode*)> bound = [this](VarNode* var) { 
        return var->GetName() == name; 
    };
    std::function<bool(VarNode*)> unbound = [this](VarNode* var) { 
        return var->GetName() != name; 
    };
    for (auto var : body->GetFreeVar() | rv::filter(bound)) {
        var->head = this;
    }
    auto to_copy = body->GetFreeVar() | rv::filter(unbound);
    std::copy(to_copy.begin(), to_copy.end(), std::back_inserter(free_var));
}

AppNode::AppNode(Node* func, Node* arg): func(func), arg(arg) {
    depth = std::max(func->GetDepth(), arg->GetDepth()) + 1;

    std::copy(func->GetFreeVar().begin(), 
    func->GetFreeVar().end(), 
    std::back_inserter(free_var));

    std::copy(arg->GetFreeVar().begin(), 
    arg->GetFreeVar().end(), 
    std::back_inserter(free_var));
}

VarNode::VarNode(std::string name): name(name) {
    free_var = {this};
}