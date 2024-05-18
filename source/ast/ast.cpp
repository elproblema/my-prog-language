#include <ast/ast.h>
#include <algorithm>
#include <ranges>

#include <iterator>
#include <functional>

namespace rv = std::ranges::views;


LambdaNode::LambdaNode(VarNode&& var, std::shared_ptr<Node> body): VarNode(std::move(var)), body(body) {
    free_var.clear();
    height = body->GetHeight() + 1;
    std::function<bool(std::weak_ptr<VarNode>)> bond = [this](std::weak_ptr<VarNode> var) { 
        return var.lock()->GetName() == name; 
    };
    std::function<bool(std::weak_ptr<VarNode>)> unbond = [this](std::weak_ptr<VarNode> var) { 
        return var.lock()->GetName() != name; 
    };
    for (auto var : body->GetFreeVar() | rv::filter(bond)) {
        bonded.push_back(var.lock());
    }
    auto to_copy = body->GetFreeVar() | rv::filter(unbond);
    std::copy(to_copy.begin(), to_copy.end(), std::back_inserter(free_var));
}

AppNode::AppNode(std::shared_ptr<Node> func, std::shared_ptr<Node> arg): func(func), arg(arg) {
    height = std::max(func->GetHeight(), arg->GetHeight()) + 1;

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


std::vector<std::shared_ptr<Node>> AppNode::GetChildren() {
    return {func, arg};
}