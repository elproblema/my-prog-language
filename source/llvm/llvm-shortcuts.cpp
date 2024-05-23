#include "llvm/g-adapter.h"
#include <cstdint>
#include <llvm-18/llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/Value.h>
#include <llvm/llvm-shortcuts.h>

llvm::Value* PointerAdd(GMachineState* state, llvm::Value* ptr, int64_t delta) {
    return state->builder.CreatePtrAdd(ptr, 
    llvm::Constant::getIntegerValue
    (llvm::Type::getInt64Ty(state->ctx), 
    llvm::APInt(64, delta, true)));
}

void MoveStackTop(GMachineState* state) {
    state->builder.CreateStore(PointerAdd(state, state->stack_top, 1), state->stack_top);
}

void StoreStackNode(GMachineState * st, llvm::Value *val, llvm::Value *ptr) {
    st->builder.CreateStore(val, ptr);
}

llvm::Value* LoadStackNode(GMachineState* st, llvm::Value* ptr) {
    return st->builder.CreateLoad(st->stack_node, ptr);
}

llvm::Value* CastToNodePtr(GMachineState* st, llvm::Value* ptr) {
    return st->builder.CreateBitCast(ptr, st->node_ptr);
}

llvm::Value* ConstantInt(GMachineState* st, int64_t value) {
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(st->ctx), value);
}

llvm::Value* ConstantDouble(GMachineState* st, double value) {
    return llvm::ConstantInt::get(llvm::Type::getDoubleTy(st->ctx), value);
}

llvm::Value* ConstantChar(GMachineState* st, char value) {
    return llvm::ConstantInt::get(llvm::Type::getInt8Ty(st->ctx), value);
}

llvm::Value* InitConstNode(GMachineState* st, int64_t val) {
    auto ptr = st->builder.CreateAlloca(st->constant_node);
    auto fieldInt = st->builder.CreateStructGEP(st->constant_node, ptr, 3);
    auto fieldTag = st->builder.CreateStructGEP(st->constant_node, ptr, 0);
    st->builder.CreateStore(ConstantInt(st, val), fieldInt);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}

llvm::Value* InitConstNode(GMachineState* st, double val) {
    auto ptr = st->builder.CreateAlloca(st->constant_node);
    auto fieldDouble = st->builder.CreateStructGEP(st->constant_node, ptr, 1);
    auto fieldTag = st->builder.CreateStructGEP(st->constant_node, ptr, 0);
    st->builder.CreateStore(ConstantDouble(st, val), fieldDouble);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}

llvm::Value* InitConstNode(GMachineState* st, char val) {
    auto ptr = st->builder.CreateAlloca(st->constant_node);
    auto fieldChar = st->builder.CreateStructGEP(st->constant_node, ptr, 2);
    auto fieldTag = st->builder.CreateStructGEP(st->constant_node, ptr, 0);
    st->builder.CreateStore(ConstantDouble(st, val), fieldChar);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}