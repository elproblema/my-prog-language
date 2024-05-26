#include <ast/SC.h>
#include <ast/ast.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <visitor.h>

#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>
#include <llvm/g-sub.h>

void FuncSubstitution::compile_definition() {
    sub_func = llvm::Function::Create(
    st->SubFuncTy,
    llvm::Function::ExternalLinkage,
    comb->GetFuncName(),
    st->module
    );
    st->sub_funcs[comb->GetFuncName()] = this;
}

void FuncSubstitution::compile_body() {
    auto entry = llvm::BasicBlock::Create(st->ctx, "entry", sub_func);
    st->builder.SetInsertPoint(entry);
    DebugPrint(st, "Got into " + comb->GetFuncName());
    std::map<std::string, size_t> symb_table;
    size_t i = 0;
    size_t n = comb->GetSomeVars().size();
    for (auto var : comb->GetSomeVars()) {
        symb_table[var.lock()->GetName()] = i++;
    }
    VisitorCompiler compiler = VisitorCompiler(GMachineState::Context(st, symb_table, n));
    comb->GetBody()->accept(compiler);
    GCode::Slide(n + 1).adapt(&compiler.c);
    GCode::Unwind().Call(st);
    DebugPrint(st, "done " + comb->GetFuncName());
    DebugPrintLL(st, "const val in done", LoadFromConstantNode(st, LoadStackNode(st, LoadStackTop(st)), _INT));
    st->builder.CreateRetVoid();
}

void VisitorCompiler::visit(const ConstNode& v) {
    auto st = c.state;
    auto value = v.GetLL();

    GCode::PushInt(value).adapt(&c);
    DebugPrint(c.state, "after pushint");
}

void VisitorCompiler::visit(const VarNode& v) {
    auto st = c.state;
    size_t pos = c.depth - c.symb_table[v.GetName()] - 1;
    GCode::Push(pos).adapt(&c);
}

void VisitorCompiler::visit(const SuperCombinator& v) {
    GCode::PushGlobal(v.GetFuncName()).adapt(&c);
}

void VisitorCompiler::visit(const AppNode& v) {
    GCode::MKAP().adapt(&c);
}

// implemented only for plus for now
void VisitorCompiler::visit(const BIFNode& v) {
    GCode::Add().adapt(&c);
}

void VisitorCompiler::visit(const LambdaNode&) {
    throw "LambdaNode in processed ast";
}

void VisitorCompiler::visit(const Node&) {
    throw "Node in processed ast";
}

VisitorCompiler::VisitorCompiler(GMachineState::Context c): c(c) {} 