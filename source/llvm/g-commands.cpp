#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>

void GCode::Pop::adapt(GMachineState::context* c) {
    auto st = c->state;
    auto val = PointerAdd(c->state, c->state->stack_top, -int64_t(cnt));
    st->builder.CreateStore(val, st->stack_top);
}

void GCode::Push::adapt(GMachineState::context* c) {
    auto st = c->state;

    auto to = PointerAdd(st, st->stack_top, 1);
    auto from = PointerAdd(st, st->stack_top, n + 1);
    
    StoreStackNode(st, LoadStackNode(st, from), to);

    MoveStackTopDown(c);
}

template<typename T>
void GCode::PushValue<T>::adapt(GMachineState::context* c) {
    auto st = c->state;
    MoveStackTopDown(c);
    LoadStackNode(st, InitConstIntNode(st, value));
}

void GCode::PushFunc::adapt(GMachineState::context* c) {
    MoveStackTopDown(c);
    auto ptr = InitFuncNode(c->state, f);
    StoreStackNode(c->state, ptr, c->state->stack_top);
}

void GCode::Update::adapt(GMachineState::context* c) {
    auto st = c->state;
    StoreStackNode(
        st, 
    LoadStackNode(st, st->stack_top), 
    PointerAdd(st, st->stack_top, n)
    );
}

void GCode::Slide::adapt(GMachineState::context* c) {
    GCode::Update(n).adapt(c);
    GCode::Pop(n).adapt(c);
}

void GCode::Eval::adapt(GMachineState* st) {
    auto AppBB = llvm::BasicBlock::Create(st->ctx, "App");
    auto ZeroCombBB = llvm::BasicBlock::Create(st->ctx, "ZeroComb");
    auto MergeBB = llvm::BasicBlock::Create(st->ctx, "Merge");

    auto SwitchValue = LoadTag(st, LoadStackNode(st, st->stack_top));
    auto Switch = st->builder.CreateSwitch(SwitchValue, MergeBB, 2);

    st->builder.SetInsertPoint(AppBB);
    Unwind().adapt(st);
    st->builder.CreateBr(MergeBB);

    st->builder.SetInsertPoint(ZeroCombBB);
    CallSubstitution(st, st->stack_top);
    st->builder.CreateBr(MergeBB);

    st->builder.SetInsertPoint(MergeBB);
}

void GCode::MKAP::adapt(GMachineState::context* c) {
    auto st = c->state;
    auto rhs = LoadStackNode(st, st->stack_top);
    auto lhs = LoadStackNode(st, PointerAdd(st, st->stack_top, -1));
    auto ptr = CreateAppNode(c->state, lhs, rhs);
    StoreStackNode(c->state, ptr, PointerAdd(st, st->stack_top, -1));
    GCode::Pop(1).adapt(c);
}

void GCode::Unwind::adapt(GMachineState* s) {
    s->builder.CreateCall(s->UnwindFunc);
}

void GCode::Add::adapt(GMachineState::context* c) {
    auto st = c->state;

    auto IntBB = llvm::BasicBlock::Create(st->ctx, "Int");
    auto DoubleBB = llvm::BasicBlock::Create(st->ctx, "Double");
    auto CharBB = llvm::BasicBlock::Create(st->ctx, "Char");
    auto MergeBB = llvm::BasicBlock::Create(st->ctx, "Merge");

    GCode::Eval().adapt(c->state);
    GCode::Push(1).adapt(c);
    GCode::Eval().adapt(c->state);
    GCode::Update(2).adapt(c);
    GCode::Pop(1).adapt(c);

    auto SwitchValue = LoadTag(st, LoadStackNode(st, st->stack_top));
    auto top_ptr = LoadStackNode(st, st->stack_top);
    auto pre_top_ptr = LoadStackNode(st, PointerAdd(st, st->stack_top, -1));
    auto Switch = st->builder.CreateSwitch(SwitchValue, MergeBB, 3);
    Switch->addCase(ConstantChar(st, CONST_INT), IntBB);
    Switch->addCase(ConstantChar(st, CONST_FLOAT), DoubleBB);
    Switch->addCase(ConstantChar(st, CONST_CHAR), CharBB);
    
    st->builder.SetInsertPoint(IntBB);
    
    auto top = LoadFromConstantNode(st, top_ptr, INT);
    auto pre_top = LoadFromConstantNode(st, pre_top_ptr, INT);
    auto res = st->builder.CreateAdd(top, pre_top);
    auto res_ptr = CreateConstNode(st, res, INT);
    StoreStackNode(st, res_ptr, PointerAdd(st, st->stack_top, -1));
    GCode::Pop(1).adapt(c);

    st->builder.SetInsertPoint(DoubleBB);
    top = LoadFromConstantNode(st, top_ptr, DOUBLE);
    pre_top = LoadFromConstantNode(st, pre_top_ptr, DOUBLE);
    res = st->builder.CreateAdd(top, pre_top);
    res_ptr = CreateConstNode(st, res, DOUBLE);
    StoreStackNode(st, res_ptr, PointerAdd(st, st->stack_top, -1));
    GCode::Pop(1).adapt(c);

    st->builder.SetInsertPoint(CharBB);
    top = LoadFromConstantNode(st, top_ptr, CHAR);
    pre_top = LoadFromConstantNode(st, pre_top_ptr, CHAR);
    res = st->builder.CreateAdd(top, pre_top);
    res_ptr = CreateConstNode(st, res, CHAR);
    StoreStackNode(st, res_ptr, PointerAdd(st, st->stack_top, -1));
    GCode::Pop(1).adapt(c);

    st->builder.SetInsertPoint(MergeBB);
}