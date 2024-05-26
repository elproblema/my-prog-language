#pragma once
#include "g-adapter.h"
#include <cstddef>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>
#include <string>

llvm::Value* PointerAdd(GMachineState*, llvm::Value*, int64_t);
void StoreStackNode(GMachineState*, llvm::Value* val, llvm::Value* ptr);
llvm::Value* LoadStackNode(GMachineState*, llvm::Value* ptr);
llvm::Value* CastToNodePtr(GMachineState*, llvm::Value*);
llvm::Value* CreateConstNode(GMachineState*, llvm::Value*, PosInConst);
llvm::Value* CreateConstNode(GMachineState*, int64_t);
llvm::Value* LoadTag(GMachineState*, llvm::Value* ptr);
llvm::ConstantInt* ConstantInt(GMachineState* st, int64_t value);
llvm::Constant* ConstantDouble(GMachineState* st, double value);
llvm::ConstantInt* ConstantChar(GMachineState* st, char value);
llvm::Value* LoadFromConstantNode(GMachineState* st, llvm::Value* ptr, PosInConst pos);
void CallSubstitution(GMachineState* st, llvm::Value* ptr);
llvm::Value* CreateAppNode(GMachineState* st, llvm::Value* lhs, llvm::Value* rhs);
llvm::Value* LoadFromAppNode(GMachineState* st, llvm::Value* ptr, size_t pos);
llvm::Value* CreateFuncNode(FuncSubstitution* fs);
llvm::Value* GetFuncPtr(GMachineState* st, llvm::Value* ptr);
llvm::Value* GetArgCnt(GMachineState* st, llvm::Value* ptr);
llvm::Value* LoadStackTop(GMachineState*);
void StoreStackTop(GMachineState*, llvm::Value*);
void CreateStackStore(GMachineState* st, llvm::Value* val, llvm::Value* ptr);
void MoveStack(GMachineState* st, llvm::Value* val);
void MoveStackConst(GMachineState::Context* c, int64_t val);
void DebugPrint(GMachineState* st, std::string msg);
// ptr is pointer to value we want to print
void DebugPrintChar(GMachineState* st, std::string msg, llvm::Value*);
void DebugPrintLL(GMachineState* st, std::string msg, llvm::Value*);