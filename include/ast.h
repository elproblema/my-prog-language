#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <variant>

class Node; // Node instance associated with some lambda expression

class VarNode; // This node stands for variable expression

class Node {
  protected:
    std::vector<VarNode*> free_var;
    size_t depth;

  public:
    size_t GetDepth() const { 
      return depth; 
    }
    const std::vector<VarNode*>& GetFreeVar() { 
      return free_var; 
    }

    Node(): depth(0), free_var() {}
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

class PrintNode : public BIFNode {};

class GetIntNode : public BIFNode {};

class GetFloatNode : public BIFNode {};

class GetCharNode : public BIFNode {};

class LambdaNode;

// Stands for "[ID]" expression, where ID is just identificator
class VarNode : public Node {
  protected:
    std::string name;
    LambdaNode* head; // pointer to LambdaNode that this variable
  
  public:
    VarNode(std::string name);
    const std::string& GetName() const { return name; }

  friend class LambdaNode;
};

// Stands for lambda abstraction expression
class LambdaNode : public VarNode {
    Node* body;    
  
  public:
    LambdaNode(VarNode&& var, Node* body);
    const Node* GetBody() const { return body; }

  friend class VarNode;
};

// Stands for application expression
class AppNode : public Node {
    Node* func; // left expression
    Node* arg; // right expression

  public:
    AppNode(Node* func, Node* arg);
    const Node* GetFunc() const { return func; }
    const Node* GetArg() const { return arg; }
};
