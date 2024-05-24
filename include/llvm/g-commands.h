#pragma once
#include "g-adapter.h"

namespace GCode {

struct Pop {
    size_t cnt;

    void adapt(GMachineState::context* c); 
};

struct Push {
    size_t n;

    void adapt(GMachineState::context* c);
};

template<typename T>
struct PushValue {
    T value;

    void adapt(GMachineState::context* c);
};

struct PushFunc {
    FuncSubstitution* f;

    void adapt(GMachineState::context* c);
};

struct Update {
    size_t n;

    void adapt(GMachineState::context* c);
};

struct Slide {
    size_t n;

    void adapt(GMachineState::context* c);
};

struct Unwind {
    void adapt(GMachineState* st);
};

struct Eval {
    void adapt(GMachineState* st);
};

struct Add {
    void adapt(GMachineState::context* c);
};

struct MKAP {
    void adapt(GMachineState::context* c);
};

}