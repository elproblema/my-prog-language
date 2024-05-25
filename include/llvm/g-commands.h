#pragma once
#include "g-adapter.h"
#include <llvm/IR/Function.h>
#include <string>

namespace GCode {

struct Pop {
    size_t cnt;

    void adapt(GMachineState::Context* c);
};

struct Push {
    size_t n;

    void adapt(GMachineState::Context* c);
};


struct PushInt {
    int64_t int_val;

    void adapt(GMachineState::Context* c);
};

struct Update {
    size_t n;

    void adapt(GMachineState::Context* c);
};

struct Slide {
    size_t n;

    void adapt(GMachineState::Context* c);
};

struct Unwind {
    static llvm::Function* UnwindFunc;
    static llvm::Function* UnpackFunc;

    void Call(GMachineState* st);
    static void Init(GMachineState* st);
};

struct Eval {
    static llvm::Function* EvalFunc;

    void Call(GMachineState* st);
    static void Init(GMachineState* st);
};

struct Add {

    void adapt(GMachineState::Context* c);
};

struct MKAP {
    void adapt(GMachineState::Context* c);
};

struct PushGlobal {
    std::string name;
    
    void adapt(GMachineState::Context* st);
};

}