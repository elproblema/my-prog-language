#pragma once

#include <vector>
#include <string>
#include <variant>

class Node; // Node instance associated with some lambda expression

class VarNode; // This node stands for variable expression

class Node {
    std::vector<VarNode*> free_var;

public:
    Node() = default;
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

// Stands for "[ID]" expression, where ID is just identificator
class VarNode : public Node {
    std::string name; 
  
  public:
    VarNode(std::string name): name(name) {}
    const std::string& GetName() const { return name; }
};

// Stands for lambda abstraction expression
class LambdaNode : public VarNode {
    Node* body;    
  
  public:
    LambdaNode(VarNode&& var, Node* body): VarNode(var), body(body) {}  
    const Node* GetBody() const { return body; }
};

// Stands for application expression
class AppNode : public Node {
    Node* func; // left expression
    Node* arg; // right expression

  public:
    AppNode(Node* func, Node* arg): func(func), arg(arg) {}
    const Node* GetFunc() const { return func; }
    const Node* GetArg() const { return arg; }
};
