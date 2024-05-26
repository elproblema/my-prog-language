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

llvm::Value* PointerAdd(GMachineState* state, llvm::Value* ptr, int64_t delta) {
    return state->builder.CreatePtrAdd(ptr, 
    ConstantInt(state, delta));
}

void StoreStackNode(GMachineState * st, llvm::Value *val, llvm::Value *ptr) {
    DebugPrint(st, "begin store stack node\n");
    st->builder.CreateStore(val, ptr);
    DebugPrint(st, "end store stack node\n");
}

llvm::Value* LoadStackTop(GMachineState* st) {
    DebugPrint(st, "LoadStackTop\n");
    return st->builder.CreateLoad(st->StackPtrTy, st->module.getNamedGlobal("stack_top"));
}

llvm::Value* LoadStackNode(GMachineState* st, llvm::Value* ptr) {
    ptr = st->builder.CreateStructGEP(st->StackNodeTy, ptr, 0);
    return st->builder.CreateLoad(st->NodePtrTy, ptr);
}

void CreateStackStore(GMachineState* st, llvm::Value* val, llvm::Value* ptr) {
    ptr = st->builder.CreateStructGEP(st->StackNodeTy, ptr, 0);
    st->builder.CreateStore(val, ptr);
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
    auto fieldInt = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, _INT, "fieldInt");
    auto fieldTag = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, TAG, "fieldTag");
    st->builder.CreateStore(ConstantInt(st, val), fieldInt);
    st->builder.CreateStore(ConstantChar(st, CONST_INT), fieldTag);
    return ptr;
}

llvm::Value* LoadFromConstantNode(GMachineState* st, llvm::Value* ptr, PosInConst pos) {
    auto ptr_to_field = st->builder.CreateStructGEP(st->ConstantNodeTy, ptr, pos);
    return st->builder.CreateLoad(st->IntTy, ptr_to_field, "load from const");
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
    return st->builder.CreateLoad(st->TagTy, ptr, "load tag");
}

llvm::Value* GetFuncPtr(GMachineState* st, llvm::Value* ptr) {
    auto ptr_to_func_addr = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 2);
    return st->builder.CreateLoad(st->SubFuncTy->getPointerTo(), ptr_to_func_addr);
}

llvm::Value* GetArgCnt(GMachineState* st, llvm::Value* ptr) {
    auto ptr_to_arg_addr = st->builder.CreateStructGEP(st->FuncNodeTy, ptr, 1);
    return st->builder.CreateLoad(st->IntTy, ptr_to_arg_addr);
}

void CallSubstitution(GMachineState* st, llvm::Value* ptr) {
    DebugPrint(st, "Calling Substitution\n");
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
    ptr = st->builder.CreateStructGEP(st->AppNodeTy, ptr, pos);
    return st->builder.CreateLoad(st->NodePtrTy, ptr, "loading app node");
}

llvm::Value* CreateFuncNode(FuncSubstitution* fs) {
    DebugPrint(fs->st, "begin CreateFuncNode\n");
    auto& bld = fs->st->builder;
    auto st = fs->st;
    auto ptr = bld.CreateAlloca(st->FuncNodeTy);
    auto ptr_to_tag = bld.CreateStructGEP(st->FuncNodeTy, ptr, 0);
    auto ptr_to_args = bld.CreateStructGEP(st->FuncNodeTy, ptr, 1);
    auto ptr_to_ptr = bld.CreateStructGEP(st->FuncNodeTy, ptr, 2);
    bld.CreateStore(ConstantChar(st, FUNC), ptr_to_tag);
    bld.CreateStore(ConstantInt(st, fs->comb->GetSomeVars().size()), ptr_to_args);
    bld.CreateStore(fs->sub_func, ptr_to_ptr);
    DebugPrint(fs->st, "end CreateFuncNode");

    return ptr;
}

void StoreStackTop(GMachineState* st, llvm::Value* val) {
    auto& bld = st->builder;
    bld.CreateStore(val, st->module.getNamedGlobal("stack_top"));
}

void MoveStack(GMachineState* st, llvm::Value* val) {
    DebugPrint(st, "start MoveStack");
    auto& bld = st->builder;
    auto ptr = LoadStackTop(st);
    ptr = bld.CreatePtrAdd(ptr, val);
    StoreStackTop(st, ptr);
    DebugPrint(st, "end MoveStack");
}

void MoveStackConst(GMachineState::Context* c, int64_t val) {
    DebugPrint(c->state, "start MoveStackConst");
    MoveStack(c->state, ConstantInt(c->state, val));
    DebugPrint(c->state, "start MoveStackConst");
    c->depth += val;
}

void DebugPrint(GMachineState *st, std::string msg) {
    auto PutsTy = 
    llvm::FunctionType::get(
        llvm::IntegerType::getInt32Ty(st->ctx),
        llvm::Type::getInt8Ty(st->ctx)->getPointerTo(),
        false
    );
    auto Puts = st->module.getOrInsertFunction("puts", PutsTy);
    auto strPtr = st->builder.CreateGlobalStringPtr(msg);
    st->builder.CreateCall(Puts, {strPtr});
}

void DebugPrintChar(GMachineState *st, std::string msg, llvm::Value *ptr) {
    auto PrintF = 
    st->module.getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(st->ctx), 
        llvm::Type::getInt8Ty(st->ctx)->getPointerTo(), true /* this is var arg func type*/) 
    );
    llvm::Constant* formatStr = st->builder.CreateGlobalStringPtr(msg + " : val is " + "%u\n", "format");
    st->builder.CreateCall(PrintF, {formatStr, ptr});
}

void DebugPrintLL(GMachineState* st, std::string msg, llvm::Value *ptr) {
        auto PrintF = 
    st->module.getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(st->ctx), 
        llvm::Type::getInt8Ty(st->ctx)->getPointerTo(), true /* this is var arg func type*/) 
    );
    llvm::Constant* formatStr = st->builder.CreateGlobalStringPtr(msg + " : val is " + "%lld\n", "format");
    st->builder.CreateCall(PrintF, {formatStr, ptr});
}