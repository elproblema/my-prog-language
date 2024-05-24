#include "g-adapter.h"
#include <cstddef>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>

llvm::Value* PointerAdd(GMachineState*, llvm::Value*, int64_t);
void StoreStackNode(GMachineState*, llvm::Value* val, llvm::Value* ptr);
llvm::Value* LoadStackNode(GMachineState*, llvm::Value* ptr);
void MoveStackTopDown(GMachineState::context*);
void MoveStackTopUp(GMachineState::context*);
void AllocInGraph(GMachineState*);
llvm::Value* CastToNodePtr(GMachineState*, llvm::Value*);
llvm::Value* CreateConstNode(GMachineState*, llvm::Value*, PosInConst);
llvm::Value* InitConstNode(GMachineState*, int64_t);
llvm::Value* InitConstNode(GMachineState*, double);
llvm::Value* InitConstNode(GMachineState*, char);
llvm::Value* InitFuncNode(GMachineState*, FuncSubstitution*);
llvm::Value* LoadConstantNode(GMachineState*, llvm::Value* ptr);
llvm::Value* LoadTag(GMachineState*, llvm::Value* ptr);
llvm::ConstantInt* ConstantInt(GMachineState* st, int64_t value);
llvm::Constant* ConstantDouble(GMachineState* st, double value);
llvm::ConstantInt* ConstantChar(GMachineState* st, char value);
llvm::Value* LoadFromConstantNode(GMachineState* st, llvm::Value* ptr, PosInConst pos);
void CallSubstitution(GMachineState* st, llvm::Value* ptr);
llvm::Value* CreateAppNode(GMachineState* st, llvm::Value* lhs, llvm::Value* rhs);
llvm::Value* LoadFromAppNode(GMachineState* st, llvm::Value* ptr, size_t pos);