#pragma once
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <map>
#include <string>

enum TAG {
    APP,
    CONST_INT,
    CONST_FLOAT,
    CONST_CHAR,
    FUNC,
    IND
};

struct GMachineState {
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    llvm::StructType* stack_node;
    llvm::PointerType* stack_ptr;
    llvm::PointerType* node_ptr;

    // this has nothing similar to llvm context
    struct context {
        GMachineState* state;
        // start of the stack for the context
        llvm::Value root;
        // (var -> pos) where pos is pointer to parameter offset relative to root on the stack
        std::map<std::string, size_t> symb_table; 
        // depth of the stack 
        size_t depth;
    };

    llvm::Type* tag;
    llvm::StructType* app_node;
    llvm::StructType* constant_node;
    llvm::StructType* func_node;
    llvm::StructType* ind_node;

    llvm::GlobalVariable* stack_top;

    void DefineTypes();

    void DefineFunctions();

    void InitGlobal();

    GMachineState();
};