#include <ast.h>
#include <vector>
// Super Combinators

class SuperCombinator : public Node {
  private:
    std::vector<std::string> var_names;
    std::vector<std::shared_ptr<VarNode>> some_vars;
    std::shared_ptr<Node> body;

    std::string create_name();

  public:
    SuperCombinator(LambdaNode);

    static std::shared_ptr<Node> Substitution(std::shared_ptr<LambdaNode>);
};