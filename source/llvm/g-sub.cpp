#include <ast/SC.h>
#include <ast/ast.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <variant>
#include <visitor.h>


#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>
#include <llvm/g-sub.h>

void FuncSubstitution::compile_definition() {
    sub_func = llvm::Function::Create(st->SubFuncTy, llvm::Function::ExternalLinkage, comb->GetFuncName());
    st->sub_funcs[comb->GetFuncName()] = this;
}

void FuncSubstitution::compile_body() {
    llvm::BasicBlock::Create(st->ctx, "entry", sub_func);
    std::map<std::string, size_t> symb_table;
    size_t i = 0;
    size_t n = comb->GetSomeVars().size();
    for (auto var : comb->GetSomeVars()) {
        symb_table[var.lock()->GetName()] = i++;
    }
    VisitorCompiler compiler = VisitorCompiler(GMachineState::Context(st, symb_table, n));
    comb->accept(compiler);
    GCode::Slide(n + 1).adapt(&compiler.c);
    GCode::Unwind().adapt(st);
}

void VisitorCompiler::visit(const ConstNode& v) {
    auto st = c.state;
    auto value = v.GetConstValue();

    if (std::holds_alternative<long long>(value)) {
        GCode::PushValue(std::get<long long>(value)).adapt_int(&c);
    } else if (std::holds_alternative<long double>(value)) {
        GCode::PushValue(std::get<long double>(value)).adapt_float(&c);
    } else if (std::holds_alternative<char>(value)) {
        GCode::PushValue(std::get<char>(value)).adapt_char(&c);
    }

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