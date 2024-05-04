#pragma once
#include <exception>

struct WrongAccessException : public std::exception {
    const char* method;

    WrongAccessException(const char* method): method(method) {}

    const char* what() const noexcept override { return method; }
};

using WAE = WrongAccessException;

struct FreeVariablesException : public std::exception {
    const char* what() const noexcept override { return "Free Variables are in the programm\n"; }
};