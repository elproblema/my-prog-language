#include "llvm/g-adapter.h"
#include <cstdint>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>
#include <llvm-c/Types.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Value.h>
#include <llvm/llvm-shortcuts.h>

llvm::Value* PointerAdd(GMachineState* state, llvm::Value* ptr, int64_t delta) {
    return state->builder.CreatePtrAdd(ptr, 
    llvm::Constant::getIntegerValue
    (llvm::Type::getInt64Ty(state->ctx), 
    llvm::APInt(64, delta, true)));
}

void MoveStackTopDown(GMachineState::context* c) {
    c->state->builder.CreateStore(PointerAdd(c->state, c->state->stack_top, 1), c->state->stack_top);
    ++c->depth;
}

void MoveStackTopUp(GMachineState::context* c) {
    c->state->builder.CreateStore(PointerAdd(c->state, c->state->stack_top, -1), c->state->stack_top);
    --c->depth;
}

void StoreStackNode(GMachineState * st, llvm::Value *val, llvm::Value *ptr) {
    st->builder.CreateStore(val, ptr);
}

llvm::Value* LoadStackNode(GMachineState* st, llvm::Value* ptr) {
    return st->builder.CreateLoad(st->StackNodeTy, ptr);
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

llvm::Value* InitConstNode(GMachineState* st, int64_t val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldInt = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, INT);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantInt(st, val), fieldInt);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}

llvm::Value* InitConstNode(GMachineState* st, double val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldDouble = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, DOUBLE);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantDouble(st, val), fieldDouble);
    st->builder.CreateStore(ConstantChar(st, CONST_FLOAT), fieldTag);
    return ptr;
}

llvm::Value* InitConstNode(GMachineState* st, char val) {
    auto ptr = st->builder.CreateAlloca(st->ConstantNodeTy);
    auto fieldChar = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, CHAR);
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG);
    st->builder.CreateStore(ConstantDouble(st, val), fieldChar);
    st->builder.CreateStore(ConstantChar(st, CONST_CHAR), fieldTag);
    return ptr;
}

llvm::Value* InitFuncNode(GMachineState* st, FuncSubstitution* f) {
    auto ptr = st->builder.CreateAlloca(st->FuncNodeTy);
    llvm::Value* TagField = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    llvm::Value* FuncField = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    st->builder.CreateStore(InitConstNode(st, FUNC), TagField);
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