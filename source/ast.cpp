#include <ast.h>
#include <algorithm>
#include <ranges>

#include <iterator>
#include <functional>

namespace rv = std::ranges::views;


LambdaNode::LambdaNode(VarNode&& var, std::shared_ptr<Node> body): VarNode(std::move(var)), body(body) {
    free_var.clear();
    depth = body->GetDepth() + 1;
    std::function<bool(std::weak_ptr<VarNode>)> bound = [this](std::weak_ptr<VarNode> var) { 
        return var.lock()->GetName() == name; 
    };
    std::function<bool(std::weak_ptr<VarNode>)> unbound = [this](std::weak_ptr<VarNode> var) { 
        return var.lock()->GetName() != name; 
    };
    for (auto var : body->GetFreeVar() | rv::filter(bound)) {
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

VarNode::VarNode(std::string name): name(name) {}

void VarNode::SetFreeVar() {
    free_var = {weak_from_this()};
}

void LambdaNode::SetBounded() {
    for (auto var : bonded) {
        var.lock()->head = weak_from_this();
    }
}

std::vector<std::shared_ptr<Node>> Node::GetChildren() {
    return {};
} 

std::vector<std::shared_ptr<Node>> LambdaNode::GetChildren() {
    return {body};
}

std::vector<std::shared_ptr<Node>> AppNode::GetChildren() {
    return {func, arg};
}