#include <cstddef>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Value.h>

#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>
#include <llvm/g-sub.h>

#include <iostream>

llvm::Value* PointerAdd(GMachineState* state, llvm::Value* ptr, int64_t delta) {
    return state->builder.CreatePtrAdd(ptr, 
    ConstantInt(state, delta));
}

void MoveStackTopDown(GMachineState::Context* c) {
    c->state->builder.CreateStore(PointerAdd(c->state, c->state->stack_top, 1), c->state->stack_top);
    ++c->depth;
}

void MoveStackTopUp(GMachineState::Context* c) {
    c->state->builder.CreateStore(PointerAdd(c->state, c->state->stack_top, -1), c->state->stack_top);
    --c->depth;
}

void StoreStackNode(GMachineState * st, llvm::Value *val, llvm::Value *ptr) {
    st->builder.CreateStore(val, ptr);
}

llvm::Value* LoadStackTop(GMachineState* st) {
    std::cout << "stack_top_load start\n";
    return st->builder.CreateLoad(st->StackPtrTy, st->stack_top, "load_stack_top");
    std::cout << "stack_top_load end\n";
}

llvm::Value* LoadStackNode(GMachineState* st, llvm::Value* ptr) {
    std::cout << "stack_node_load start\n";
    return st->builder.CreateStructGEP(st->StackNodeTy, ptr, 0, "load_stack_node");
}

void CreateStackStore(GMachineState* st, llvm::Value* val, llvm::Value* ptr) {
    std::cout << "stack_node_store start\n";
    ptr = st->builder.CreateStructGEP(st->StackNodeTy, ptr, 0);
    st->builder.CreateStore(val, ptr);
    std::cout << "stack_node_store end\n";
}

llvm::Value* CastToNodePtr(GMachineState* st, llvm::Value* ptr) {
    return st->builder.CreateBitCast(ptr, st->NodePtrTy);
}

llvm::ConstantInt* ConstantInt(GMachineState* st, int64_t value) {
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(st->ctx), value);
}

llvm::Constant* ConstantDouble(GMachineState* st, double value) {
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(st->ctx), value);
}

llvm::ConstantInt* ConstantChar(GMachineState* st, char value) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(st->ctx), value);
}

llvm::Value* CreateConstNode(GMachineState* st, int64_t val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldInt = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, _INT);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantInt(st, val), fieldInt);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}

llvm::Value* CreateConstNode(GMachineState* st, long double val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldDouble = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, _DOUBLE);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantDouble(st, val), fieldDouble);
    st->builder.CreateStore(ConstantChar(st, CONST_FLOAT), fieldTag);
    return ptr;
}

llvm::Value* CreateConstNode(GMachineState* st, char val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldChar = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, _CHAR);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantDouble(st, val), fieldChar);
    st->builder.CreateStore(ConstantChar(st, CONST_CHAR), fieldTag);
    return ptr;
}

llvm::Value* InitFuncNode(GMachineState* st, FuncSubstitution* f) {
    auto ptr = st->builder.CreateAlloca(st->FuncNodeTy);
    llvm::Value* TagField = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    llvm::Value* FuncField = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    st->builder.CreateStore(CreateConstNode(st, FUNC), TagField);
    llvm::Value* 
    int_ptr = st->builder.CreateBitCast(
        f->sub_func, 
        llvm::PointerType::getUnqual(llvm::Type::getInt32Ty(st->ctx))
    );
    st->builder.CreateStore(int_ptr, FuncField);
    return ptr;
}

llvm::Value* LoadFromConstantNode(GMachineState* st, llvm::Value* ptr, PosInConst pos) {
    return st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, pos);
}

llvm::Value* CreateConstNode(GMachineState* st, llvm::Value* to_load, PosInConst pos) {
    auto allocated = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto ptr_to_load = st->builder.CreateStructGEP(st->ConstantNodeTy, allocated, pos);
    auto ptr_to_tag = st->builder.CreateStructGEP(st->ConstantNodeTy, allocated, 0);
    st->builder.CreateStore(to_load, ptr_to_load);
    st->builder.CreateStore(ConstantInt(st, CONST_INT + char(pos) - _INT), ptr_to_tag);
    return allocated;
}

llvm::Value* LoadTag(GMachineState* st, llvm::Value* ptr) {
    return st->builder.CreateLoad(st->TagTy, ptr, "loading tag");
}

llvm::Value* GetFuncPtr(GMachineState* st, llvm::Value* ptr) {
    auto ptr_to_func_addr = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 2);
    return st->builder.CreateLoad(st->SubFuncTy, ptr_to_func_addr);
}

llvm::Value* GetArgCnt(GMachineState* st, llvm::Value* ptr) {
    auto ptr_to_func_addr = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 1);
    return st->builder.CreateLoad(st->IntTy, ptr_to_func_addr);
}

void CallSubstitution(GMachineState* st, llvm::Value* ptr) {
    auto func_addr = GetFuncPtr(st, ptr);
    st->builder.CreateCall(st->SubFuncTy, func_addr);
}

llvm::Value* CreateAppNode(GMachineState* st, llvm::Value* lhs, llvm::Value* rhs) {
    auto ptr = st->builder.CreateAlloca(st->AppNodeTy, nullptr, "app node");
    auto ptr_to_tag = st->builder.CreateStructGEP(st->AppNodeTy, ptr, 0);
    auto ptr_to_lhs = st->builder.CreateStructGEP(st->AppNodeTy, ptr, 1);
    auto ptr_to_rhs = st->builder.CreateStructGEP(st->AppNodeTy, ptr, 2);
    st->builder.CreateStore(ptr_to_lhs, lhs, "store lhs");
    st->builder.CreateStore(ptr_to_rhs, rhs, "store rhs");
    st->builder.CreateStore(ptr_to_tag, ConstantChar(st, APP), "store tag");
    return ptr;
}

llvm::Value* LoadFromAppNode(GMachineState* st, llvm::Value* ptr, size_t pos) {
    ptr = st->builder.CreatePointerCast(ptr, st->AppNodeTy->getPointerTo());
    return st->builder.CreateStructGEP(st->AppNodeTy, ptr, pos);
}

llvm::Value* CreateFuncNode(FuncSubstitution* fs) {
    auto& bld = fs->st->builder;
    auto st = fs->st;
    auto ptr = bld.CreateAlloca(st->FuncNodeTy, nullptr, "func");
    auto ptr_to_tag = bld.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    auto ptr_to_args = bld.CreateStructGEP(st->FuncNodeTy, ptr, 1);
    auto ptr_to_ptr = bld.CreateStructGEP(st->FuncNodeTy, ptr, 2);
    bld.CreateStore(ptr_to_tag, ConstantChar(st, FUNC), "store tag");
    bld.CreateStore(ptr_to_args, ConstantInt(st, fs->comb->GetSomeVars().size()));
    bld.CreateStore(ptr_to_ptr, fs->sub_func);

    return ptr;
}

void StoreStackTop(GMachineState* st, llvm::Value* val) {
    auto& bld = st->builder;
    bld.CreateLoad(st->StackPtrTy, val);
}

void MoveStack(GMachineState* st, llvm::Value* val) {
    auto& bld = st->builder;
    auto ptr = LoadStackTop(st);
    ptr = bld.CreateAdd(ptr, val);
    StoreStackTop(st, ptr);
}

void MoveStackConst(GMachineState::Context* c, int64_t val) {
    MoveStack(c->state, ConstantInt(c->state, val));
    c->depth += val;
}