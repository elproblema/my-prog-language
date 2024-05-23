#include "g-adapter.h"
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Value.h>

llvm::Value* PointerAdd(GMachineState*, llvm::Value*, int64_t);
void StoreStackNode(GMachineState*, llvm::Value* val, llvm::Value* ptr);
llvm::Value* LoadStackNode(GMachineState*, llvm::Value* ptr);
void MoveStackTop(GMachineState*);
void AllocInGraph(GMachineState*);
llvm::Value* CastToNodePtr(GMachineState*, llvm::Value*);
llvm::Value* InitConstIntNode(GMachineState*, int64_t);