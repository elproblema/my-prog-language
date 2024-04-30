#pragma once

#include <ast.h>
#include <vector>
// Super Combinators

class SuperCombinator : public Node {
  protected:
    std::vector<std::weak_ptr<VarNode>> some_vars;
    std::shared_ptr<Node> body;
    std::string name;

    std::string create_name();

    std::shared_ptr<SuperCombinator> eta_conversion();

    std::shared_ptr<SuperCombinator> shared_from_this() { 
        return std::dynamic_pointer_cast<SuperCombinator>(Node::shared_from_this());
    }

    std::shared_ptr<const SuperCombinator> shared_from_this() const { 
        return std::dynamic_pointer_cast<const SuperCombinator>(Node::shared_from_this());
    }
  public:
    SuperCombinator(LambdaNode);

    SuperCombinator(std::shared_ptr<Node> body, std::string name): body(body), name(name) {}

    std::shared_ptr<const Node> GetBody() const { return body; }

    static std::shared_ptr<Node> Substitution(std::shared_ptr<LambdaNode>);
};

using SC_container = std::vector<std::shared_ptr<SuperCombinator>>;

static SC_container all_functions;

SC_container GetFromAst(std::shared_ptr<Node> root);