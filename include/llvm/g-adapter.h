#pragma once
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <map>
#include <string>

#include <ast/SC.h>

enum TAG : char {
    APP,
    CONST_INT,
    CONST_FLOAT,
    CONST_CHAR,
    FUNC
};

enum PosInConst : uint32_t {
    TAG=0,
    _INT,
    _DOUBLE,
    _CHAR,
};

struct FuncSubstitution;

using FS_container = std::map<std::string, FuncSubstitution*>;

struct GMachineState {
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    llvm::Type* VoidTy;
    llvm::Type* IntTy;
    llvm::Type* DoubleTy;
    llvm::Type* CharTy;
    llvm::FunctionType* SubFuncTy;

    llvm::StructType* StackNodeTy;
    llvm::PointerType* StackPtrTy;
    llvm::PointerType* NodePtrTy;

    llvm::Function* UnwindFunc;
    llvm::Function* UnpackFunc;
    llvm::Function* Main;

    // this has nothing similar to llvm context
    struct Context {
        GMachineState* state;
        // (var -> pos) where pos is pointer to parameter offset relative to root on the stack
        std::map<std::string, size_t> symb_table; 
        // depth of the stack 
        size_t depth;
    };

    llvm::Type* TagTy;
    llvm::StructType* AppNodeTy;
    llvm::StructType* ConstantNodeTy;
    llvm::StructType* FuncNodeTy;
    llvm::StructType* IndNodeTy;

    llvm::GlobalVariable* stack_top;

    FS_container sub_funcs;

    void DefineTypes();

    void DefineFunctions();

    void InitGlobal();

    GMachineState();
};