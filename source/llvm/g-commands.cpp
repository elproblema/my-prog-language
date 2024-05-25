#include <llvm/IR/Function.h>
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
#include <llvm/g-sub.h>

void GCode::Pop::adapt(GMachineState::Context* c) {
    auto st = c->state;
    auto val = PointerAdd(c->state, c->state->stack_top, -int64_t(cnt));
    st->builder.CreateStore(val, st->stack_top);
}

void GCode::PushInt::adapt(GMachineState::Context* c) {
    auto st = c->state;
    MoveStackConst(c, 1);
    StoreStackNode(st, CreateConstNode(st, int_val), st->stack_top);
}

void GCode::Update::adapt(GMachineState::Context* c) {
    auto st = c->state;
    StoreStackNode(
        st,
    LoadStackNode(st, LoadStackTop(st)), 
    PointerAdd(st, LoadStackTop(st), n)
    );
}

void GCode::Slide::adapt(GMachineState::Context* c) {
    GCode::Update(n).adapt(c);
    GCode::Pop(n).adapt(c);
}

void GCode::Eval::Init(GMachineState* st) {
    EvalFunc = llvm::Function::Create(
        llvm::FunctionType::get(st->VoidTy, false), 
        llvm::Function::ExternalLinkage,
        "Eval",
        st->module
    );

    auto EntryBB = llvm::BasicBlock::Create(st->ctx, "entry", EvalFunc);
    auto AppBB = llvm::BasicBlock::Create(st->ctx, "app", EvalFunc);
    auto FuncBB = llvm::BasicBlock::Create(st->ctx, "func", EvalFunc);
    auto ZeroCombBB = llvm::BasicBlock::Create(st->ctx, "zero_arg", EvalFunc);
    auto MergeBB = llvm::BasicBlock::Create(st->ctx, "merge", EvalFunc);

    auto& bld = st->builder;
    bld.SetInsertPoint(EntryBB);

    auto SwitchValue = LoadTag(st, LoadStackNode(st, LoadStackTop(st)));
    auto Switch = st->builder.CreateSwitch(SwitchValue, MergeBB, 2);
    Switch->addCase(ConstantInt(st, APP), AppBB);
    Switch->addCase(ConstantInt(st, FUNC), FuncBB);

    st->builder.SetInsertPoint(AppBB);
    Unwind().Call(st);
    st->builder.CreateBr(MergeBB);

    st->builder.SetInsertPoint(FuncBB);
    auto arg_cnt = GetArgCnt(st, LoadStackNode(st, LoadStackTop(st)));
    auto cond = st->builder.CreateICmpEQ(arg_cnt, ConstantInt(st, 0));
    st->builder.CreateCondBr(cond, ZeroCombBB, MergeBB);

    st->builder.SetInsertPoint(ZeroCombBB);
    CallSubstitution(st, LoadStackNode(st, LoadStackTop(st)));
    st->builder.CreateBr(MergeBB);

    st->builder.SetInsertPoint(MergeBB);
    st->builder.CreateRetVoid();
}

void GCode::MKAP::adapt(GMachineState::Context* c) {
    auto st = c->state;
    auto rhs = LoadStackNode(st, LoadStackTop(st));
    auto lhs = LoadStackNode(st, PointerAdd(st, LoadStackTop(st), -1));
    auto ptr = CreateAppNode(c->state, lhs, rhs);
    StoreStackNode(c->state, ptr, PointerAdd(st, LoadStackTop(st), -1));
    GCode::Pop(1).adapt(c);
}

void GCode::Unwind::Call(GMachineState* st) {
    st->builder.CreateCall(st->UnwindFunc);
}

void GCode::Unwind::Init(GMachineState *st) {
    {
        auto FuncTy = llvm::FunctionType::get(st->VoidTy, false);
        UnwindFunc = llvm::Function::Create(FuncTy, llvm::Function::ExternalLinkage, "unwind", st->module);
    }
    {
        auto FuncTy = llvm::FunctionType::get(st->VoidTy, st->IntTy, false);
        UnpackFunc = llvm::Function::Create(FuncTy, llvm::Function::ExternalLinkage, "unpack", st->module);
    }
    auto& bld = st->builder;
    // UNWIND START
    {
        auto entry = llvm::BasicBlock::Create(st->ctx, "entry", UnwindFunc);

        bld.SetInsertPoint(entry);
        auto AppBB = llvm::BasicBlock::Create(st->ctx, "app_node", UnwindFunc);
        auto FuncBB = llvm::BasicBlock::Create(st->ctx, "func_node", UnwindFunc);
        auto MergeBB = llvm::BasicBlock::Create(st->ctx, "merge", UnwindFunc);

        auto SwitchValue = LoadTag(st, LoadStackTop(st));
        auto Switch = bld.CreateSwitch(SwitchValue, MergeBB, 2);
        Switch->addCase(ConstantChar(st, APP), AppBB);
        Switch->addCase(ConstantChar(st, FUNC), FuncBB);

        bld.SetInsertPoint(AppBB);
        auto lhs = LoadFromAppNode(st, LoadStackNode(st, LoadStackTop(st)), 1);
        MoveStack(st, ConstantInt(st, 1));
        bld.CreateStore(lhs, LoadStackNode(st, LoadStackTop(st)));
        bld.CreateBr(MergeBB);

        bld.SetInsertPoint(FuncBB);
        auto func_ptr = GetFuncPtr(st, LoadStackNode(st, LoadStackTop(st)));
        auto val = GetArgCnt(st, LoadStackNode(st, LoadStackTop(st)));
        bld.CreateCall(UnpackFunc, val);
        bld.CreateCall(llvm::FunctionType::get(st->VoidTy, false), func_ptr);
        bld.CreateBr(MergeBB);

        bld.SetInsertPoint(MergeBB);
        bld.CreateRetVoid();
    }
    //UNWIND END
    //UNPACK START
    {
        auto entry = llvm::BasicBlock::Create(st->ctx, "entry", UnpackFunc);
        auto loop_block = llvm::BasicBlock::Create(st->ctx, "loop_label", UnpackFunc);
        auto ret_block = llvm::BasicBlock::Create(st->ctx, "ret_label", UnpackFunc);

        bld.SetInsertPoint(entry);
        auto ptr_to_ind = bld.CreateAlloca(st->IntTy, nullptr, "ind");
        bld.CreateStore(ConstantInt(st, 0), ptr_to_ind);
        bld.CreateBr(loop_block);

        bld.SetInsertPoint(ret_block);
        bld.CreateRetVoid();

        bld.SetInsertPoint(loop_block);
        auto ptr_to_node_on_ind = bld.CreatePtrAdd(LoadStackTop(st), bld.CreateLoad(st->IntTy, ptr_to_ind));
        auto prev_ptr_to_node_on_ind = PointerAdd(st, ptr_to_node_on_ind, -1);
        StoreStackNode(st, LoadFromAppNode(st, prev_ptr_to_node_on_ind, 2), ptr_to_node_on_ind);
        bld.CreateStore(bld.CreateAdd(bld.CreateLoad(st->IntTy, ptr_to_ind), ConstantInt(st, 1)), ptr_to_ind);
        auto cond = bld.CreateICmpSLT(bld.CreateLoad(st->IntTy, ptr_to_ind), bld.CreateLoad(st->IntTy, &(*UnwindFunc->arg_begin())));
        bld.CreateCondBr(cond, loop_block, ret_block);
    }
    //UNPACK END
}

void GCode::Add::adapt(GMachineState::Context* c) {
    auto st = c->state;

    auto top_ptr = LoadStackNode(st, st->stack_top);
    auto pre_top_ptr = LoadStackNode(st, PointerAdd(st, st->stack_top, -1));
        
    auto top = LoadFromConstantNode(st, top_ptr, _INT);
    auto pre_top = LoadFromConstantNode(st, pre_top_ptr, _INT);
    auto res = st->builder.CreateAdd(top, pre_top);
    auto res_ptr = CreateConstNode(st, res, _INT);
    StoreStackNode(st, res_ptr, PointerAdd(st, st->stack_top, -1));
    GCode::Pop(1).adapt(c);
}

void GCode::PushGlobal::adapt(GMachineState::Context* c) {
    auto st = c->state;
    FuncSubstitution* fs = st->sub_funcs[name];
    MoveStackConst(c, 1);
    StoreStackNode(st, CreateFuncNode(fs), LoadStackTop(st));
}