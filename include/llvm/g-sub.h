#pragma once
#include <ast/SC.h>
#include <ast/ast.h>
#include <visitor.h>
#include <memory>
#include <llvm/g-adapter.h>

//this struct contains of function that can create body of SuperCombinator comb
//with given parameteres on the stack
struct FuncSubstitution {
    GMachineState* st;
    std::shared_ptr<SuperCombinator> comb;
    llvm::Function* sub_func;

    FuncSubstitution(GMachineState* st, std::shared_ptr<SuperCombinator> comb):
    st(st), comb(comb) {}

    void compile_body();

    void compile_definition();
};

struct VisitorCompiler : public Visitor {
    GMachineState::Context c;

    void visit(const AppNode&) override;
    void visit(const SuperCombinator&) override;
    void visit(const VarNode&) override;
    void visit(const ConstNode&) override;
    void visit(const LambdaNode&) override;
    void visit(const BIFNode&) override;
    void visit(const Node&) override;
    bool skip_ind_nodes() override { return true;}

    VisitorCompiler(GMachineState::Context c);
};
