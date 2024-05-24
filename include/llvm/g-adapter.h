#pragma once
#include <llvm-18/llvm/IR/Function.h>
#include <llvm-18/llvm/IR/GlobalVariable.h>
#include <llvm-18/llvm/IR/Type.h>
#include <llvm-18/llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <map>
#include <memory>
#include <string>

#include <ast/SC.h>

enum TAG : char {
    APP,
    CONST_INT,
    CONST_FLOAT,
    CONST_CHAR,
    FUNC,
    VAR,
    IND
};

enum PosInConst : uint32_t {
    TAG=0,
    INT,
    DOUBLE,
    CHAR,
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

    llvm::StructType* StackNodeTy;
    llvm::PointerType* StackPtrTy;
    llvm::PointerType* NodePtrTy;

    llvm::Function* UnwindFunc;
    llvm::Function* UnpackFunc;

    // this has nothing similar to llvm context
    struct context {
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

    void PrepareToExecute();

    GMachineState();
};

//this struct contains of function that can create body of SuperCombinator comb
//with given parameteres on the stack
struct FuncSubstitution {
    GMachineState* st;
    std::shared_ptr<SuperCombinator> comb;
    llvm::FunctionType* sub_func_type;
    llvm::Function* sub_func;

    FuncSubstitution(GMachineState* st, std::shared_ptr<SuperCombinator> comb):
    st(st), comb(comb), 
    sub_func_type(llvm::FunctionType::get(llvm::Type::getVoidTy(st->ctx), {}, false)),
    sub_func(llvm::Function::Create(sub_func_type, llvm::Function::ExternalLinkage, comb->GetName())) {}

    void compile_body();
};