#include <ast.h>
#include <algorithm>
#include <ranges>
#include <iostream>
#include <iterator>

namespace rv = std::ranges::views;


LambdaNode::LambdaNode(VarNode&& var, std::shared_ptr<Node> body): VarNode(std::move(var)), body(body) {
    free_var.clear();
    depth = body->GetDepth() + 1;
    std::function<bool(std::shared_ptr<VarNode>)> bound = [this](std::shared_ptr<VarNode> var) { 
        return var->GetName() == name; 
    };
    std::function<bool(std::shared_ptr<VarNode>)> unbound = [this](std::shared_ptr<VarNode> var) { 
        return var->GetName() != name; 
    };
    for (auto var : body->GetFreeVar() | rv::filter(bound)) {
        var->head = shared_from_this();
        bonded.push_back(var);
    }
    auto to_copy = body->GetFreeVar() | rv::filter(unbound);
    std::copy(to_copy.begin(), to_copy.end(), std::back_inserter(free_var));
}

AppNode::AppNode(std::shared_ptr<Node> func, std::shared_ptr<Node> arg): func(func), arg(arg) {
    depth = std::max(func->GetDepth(), arg->GetDepth()) + 1;

    std::copy(func->GetFreeVar().begin(), 
    func->GetFreeVar().end(), 
    std::back_inserter(free_var));

    std::copy(arg->GetFreeVar().begin(), 
    arg->GetFreeVar().end(), 
    std::back_inserter(free_var));
}

std::shared_ptr<LambdaNode> LambdaNode::shared_from_this() {
    return std::dynamic_pointer_cast<LambdaNode>(shared_from_this());
}

VarNode::VarNode(std::string name): name(name) {
    free_var = {shared_from_this()};
}