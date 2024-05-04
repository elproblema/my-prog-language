#include <visitor.h>
#include <ast.h>
#include <SC.h>
#include <iostream>

struct Printer : public Visitor {
    void visit(const Node&) override {
        std::cout << "down" << std::endl;
        std::cout << "unknown node" << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const VarNode& v) override {
        std::cout << "down" << std::endl;
        std::cout << "VarNode with name = " << v.GetName() << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const LambdaNode& v) override {
        std::cout << "down" << std::endl;
        std::cout << "LambdaNode with ind_tag = " << v.GetIndTag() << std::endl;
        auto ptr = v.body;
        if (ptr) ptr->accept(*this);
        else std::cout << "nullptr in Lambda" << std::endl;
        std::cout << "LambdaNode with ind_tag = " << v.GetIndTag() << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const SuperCombinator& v) override {
        std::cout << "down" << std::endl;
        std::cout << "SuperCombinator with func_name = " << v.GetFuncName() << std::endl;
        auto ptr = v.GetBody();
        if (ptr) ptr->accept(*this);
        std::cout << "SuperCombinator with func_name = " << v.GetFuncName() << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const BIFNode&) override {
        std::cout << "down" << std::endl;
        std::cout << "BIFNode" << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const AppNode& v) override {
        std::cout << "down" << std::endl;
        std::cout << "AppNode" << std::endl;
        auto ptr = v.GetFunc();
        if (ptr) ptr->accept(*this);
        std::cout << "AppNode" << std::endl;
        ptr = v.GetArg();
        if (ptr) ptr->accept(*this);
        std::cout << "AppNode" << std::endl;
        std::cout << "up" << std::endl;
    }
    void visit(const ConstNode& v) override {
        std::cout << "down" << std::endl;
        std::cout << "ConstNode\n" << std::endl;
        std::cout << "up" << std::endl;
    }
};