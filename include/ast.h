#pragma once

#include <memory>
#include <vector>
#include <string>
#include <variant>

// from flex
extern int yycolumn; 
extern int yylineno; 

class VarNode; 

// Node instance associated with some lambda expression
class Node : public std::enable_shared_from_this<Node> {

class Position {
    size_t line;
    size_t col;
  
  public:
    Position(): line(yylineno), col(yycolumn) {}
    int GetLine() { return line; }
    int GetColumn() { return col; }
};

  protected:
    std::vector<std::weak_ptr<VarNode>> free_var;
    size_t depth;
    Position loc; // location of the beginning of the expression


  public:
    Node(): depth(0), free_var(), loc() {}

    size_t GetDepth() const { return depth; }

    const std::vector<std::weak_ptr<VarNode>>& GetFreeVar() const { return free_var; }

    void SetLoc(std::shared_ptr<Node> first) { loc = first->loc; }

    // Virtual functions

    virtual std::vector<std::shared_ptr<Node>> GetChildren();

    virtual std::shared_ptr<const Node> GetFunc() const { return nullptr; }
    virtual std::shared_ptr<Node> GetFunc() { return nullptr; }

    virtual std::shared_ptr<const Node> GetArg() const { return nullptr; }
    virtual std::shared_ptr<Node> GetArg() { return nullptr; }

    virtual std::shared_ptr<const Node> GetBody() const { return nullptr; }
    virtual std::shared_ptr<Node> GetBody() { return nullptr; }

    virtual void SetFreeVar() {}

    virtual void SetBounded() {}


    const std::string& GetName() const { throw; }

    virtual void Substitute();

    // End of virtual functions

    virtual ~Node() = default;
};

// Associated with expression that consists of constant
class ConstNode : public Node {
    using value_type = std::variant<long long, long double, char>;
    // There "value" is semantic value of constant expression
    value_type value;

  public:
    ConstNode(long long x): value(x) {}
    ConstNode(long double x): value(x) {}
    ConstNode(char c): value(c) {}

    const long long& GetLL() const { return std::get<long long>(value); }
    const long double& GetLD() const { return std::get<long double>(value); }
    const char& GetChar() const { return std::get<char>(value); }

    ~ConstNode() = default;
};

// This nodes associated with Built-In Functions (briefly BIF) like "+", "-", "CONS" etc.
class BIFNode : public Node {};

//Addition
class AddNode : public BIFNode {};

//Subtraction
class SubNode : public BIFNode {};

// Remainder of division
class RemNode : public BIFNode {};

// Division
class DivNode : public BIFNode {};

// Multiplication
class MulNode : public BIFNode {};

class ConsNode : public BIFNode {};

class HeadNode : public BIFNode {};

class TailNode : public BIFNode {};

class InputNode : public BIFNode {};

class PrintNode : public BIFNode {};

class GetIntNode : public BIFNode {};

class GetFloatNode : public BIFNode {};

class GetCharNode : public BIFNode {};

class LambdaNode;

// Associated with "[ID]" expression
class VarNode : public Node {
    
    std::shared_ptr<VarNode> shared_from_this() { 
        return std::dynamic_pointer_cast<VarNode>(Node::shared_from_this());
    }

    std::weak_ptr<VarNode> weak_from_this() {
        return std::weak_ptr(shared_from_this());
    }

  protected:
    std::string name;
    std::weak_ptr<LambdaNode> head; // pointer to LambdaNode that has this variable
  
  public:
    VarNode(std::string name);
    void SetFreeVar() override;
    const std::string& GetName() const { return name; }

friend class LambdaNode;
};

// Associated with lambda abstraction expression
class LambdaNode : public VarNode {
    std::shared_ptr<Node> body;   
    std::vector<std::weak_ptr<VarNode>> bonded;

    std::shared_ptr<LambdaNode> shared_from_this() { 
        return std::dynamic_pointer_cast<LambdaNode>(VarNode::shared_from_this());
    }

    std::weak_ptr<LambdaNode> weak_from_this() {
        return std::weak_ptr(shared_from_this());
    }
    
  public:
    LambdaNode(VarNode&& var, std::shared_ptr<Node> body);

    std::shared_ptr<const Node> GetBody() const override { return body; }
    std::shared_ptr<Node> GetBody() override { return body; }


    std::vector<std::shared_ptr<Node>> GetChildren() override;

    void SetBounded() override;

    void Substitute() override;


friend class VarNode;
friend class SuperCombinator;
};

// Associated with application expression
class AppNode : public Node {
    std::shared_ptr<Node> func; // left expression
    std::shared_ptr<Node> arg; // right expression

    std::shared_ptr<AppNode> shared_from_this() { 
        return std::dynamic_pointer_cast<AppNode>(Node::shared_from_this());
    }

  public:
    AppNode(std::shared_ptr<Node> func, std::shared_ptr<Node> arg);

    std::shared_ptr<const Node> GetFunc() const override { return func; }
    std::shared_ptr<Node> GetFunc() override { return func; }

    std::shared_ptr<const Node> GetArg() const override { return arg; }
    std::shared_ptr<Node> GetArg() override { return arg; }

    std::vector<std::shared_ptr<Node>> GetChildren() override;
};
