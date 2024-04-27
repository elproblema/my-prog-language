#pragma once
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <variant>

extern int yycolumn;
extern int yylineno; // from flex

class VarNode; // This node stands for variable expression

// Node instance associated with some lambda expression
class Node {

class Position {
    size_t line;
    size_t col;
  
  public:
    Position(): line(yylineno), col(yycolumn) {}
    int GetLine() { return line; }
    int GetColumn() { return col; }
};

  protected:
    std::vector<std::shared_ptr<VarNode>> free_var;
    size_t depth;
    Position loc; // location of the beginning of the expression

  public:
    size_t GetDepth() const { 
      return depth; 
    }

    const std::vector<std::shared_ptr<VarNode>>& GetFreeVar() { 
      return free_var; 
    }

    void SetLoc(std::shared_ptr<Node> first) {
      loc = first->loc;
    }

    Node(): depth(0), free_var(), loc() {}
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

    const value_type& GetValue() { return value; }

    ~ConstNode() = default;
};

// This nodes stands for Built-In Functions (briefly BIF) like "+", "-", "CONS" etc.
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

// Stands for "[ID]" expression, where ID is just identificator
class VarNode : public std::enable_shared_from_this<VarNode>, public Node {
  protected:
    std::string name;
    std::shared_ptr<LambdaNode> head; // pointer to LambdaNode that this variable
  
  public:
    VarNode(std::string name);
    const std::string& GetName() const { return name; }

friend class LambdaNode;
};

// Stands for lambda abstraction expression
class LambdaNode : public VarNode {
  protected:
    std::shared_ptr<Node> body;   
    std::vector<std::shared_ptr<VarNode>> bonded;

    std::shared_ptr<LambdaNode> shared_from_this();
  public:
    LambdaNode(VarNode&& var, std::shared_ptr<Node> body);
    std::shared_ptr<const Node> GetBody() const { return body; }

friend class VarNode;
friend class SuperCombinator;
};

// Stands for application expression
class AppNode : public Node {
    std::shared_ptr<Node> func; // left expression
    std::shared_ptr<Node> arg; // right expression

  public:
    AppNode(std::shared_ptr<Node> func, std::shared_ptr<Node> arg);
    std::shared_ptr<const Node> GetFunc() const { return func; }
    std::shared_ptr<const Node> GetArg() const { return arg; }
};
