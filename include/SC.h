#pragma once

#include <ast.h>
#include <vector>
// Super Combinator

class SuperCombinator : public Node {
  protected:
    std::vector<std::weak_ptr<VarNode>> some_vars;
    std::shared_ptr<Node> body;
    std::string func_name;

    std::shared_ptr<SuperCombinator> shared_from_this() { 
        return std::dynamic_pointer_cast<SuperCombinator>(Node::shared_from_this());
    }

    std::shared_ptr<const SuperCombinator> shared_from_this() const { 
        return std::dynamic_pointer_cast<const SuperCombinator>(Node::shared_from_this());
    }
  public:
    SuperCombinator(LambdaNode);
    SuperCombinator(std::shared_ptr<Node> body, std::string func_name): 
    body(body), func_name(func_name) {}

    std::shared_ptr<const Node> GetBody() const override { return body; }

    const std::vector<std::weak_ptr<VarNode>>& 
    GetSomeVars() const override;

    const std::string& GetFuncName() const override;

    bool IsFunc() const override;
    std::shared_ptr<Node> EtaConversion() override;


    static std::shared_ptr<Node> Substitution(std::shared_ptr<LambdaNode>);
};

using SC_container = std::vector<std::shared_ptr<SuperCombinator>>;

void RebuildAst(std::shared_ptr<Node>, bool = true, std::string = "PROG");