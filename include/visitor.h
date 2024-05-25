#pragma once

class Node;
class LambdaNode;
class SuperCombinator;
class VarNode;
class BIFNode;
class AppNode;
class ConstNode;

struct Visitor {
    virtual void visit(const Node&) = 0;
    virtual void visit(const LambdaNode&) = 0;
    virtual void visit(const SuperCombinator&)= 0;
    virtual void visit(const VarNode&) = 0;
    virtual void visit(const BIFNode&) = 0;
    virtual void visit(const AppNode&) = 0;
    virtual void visit(const ConstNode&) = 0;
    virtual bool skip_ind_nodes() = 0;
};