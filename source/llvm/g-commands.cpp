#include "llvm/g-adapter.h"
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>

void GCode::PopInstruction::adapt(GMachineState::context c) {
    PointerAdd(c.state, c.state->stack_top, -int64_t(cnt));
}

void GCode::PushInstruction::adapt(GMachineState::context c) {
    auto st = c.state;

    auto to = PointerAdd(st, st->stack_top, 1);
    auto from = PointerAdd(st, st->stack_top, pos + 1);
    
    StoreStackNode(st, LoadStackNode(st, from), to);

    MoveStackTop(st);
}

template<typename T>
void GCode::PushValue<T>::adapt(GMachineState::context c) {
    auto st = c.state;
    MoveStackTop(st);
    LoadStackNode(st, InitConstIntNode(st, value));
}