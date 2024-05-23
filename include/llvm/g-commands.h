#pragma once
#include "g-adapter.h"
#include <cstddef>

namespace GCode {

struct instruction {
    virtual void adapt(GMachineState::context c) = 0;
};

struct PopInstruction : instruction {
    size_t cnt;

    void adapt(GMachineState::context c); 
};

struct PushInstruction : instruction {
    // zero-indexed
    size_t pos;

    void adapt(GMachineState::context c);
};

template<typename T>
struct PushValue {
    T value;

    void adapt(GMachineState::context c);
};

}