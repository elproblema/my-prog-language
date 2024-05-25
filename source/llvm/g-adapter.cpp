#include "ast/SC.h"
#include <llvm-18/llvm/IR/BasicBlock.h>
#include <llvm-18/llvm/IR/Function.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>
#include <llvm/g-sub.h>

#include <iostream>

extern SC_container all_functions;

GMachineState::GMachineState(): 
builder(ctx), module("main", ctx)
{
    std::cout << "#1\n";
    DefineTypes();
    std::cout << "#2\n";
    for (auto f : all_functions) std::cout << f->GetFuncName() << "\n";
    for (auto f : all_functions) {
        sub_funcs[f->GetFuncName()] = new FuncSubstitution(this, f);
    }
    std::cout << "#3\n";
    InitGlobal();
    std::cout << "#4\n";
    DefineFunctions();
    std::cout << "#5\n";
}

void GMachineState::DefineTypes() {
    VoidTy = llvm::Type::getVoidTy(ctx);
    DoubleTy = llvm::Type::getDoubleTy(ctx);
    CharTy = llvm::Type::getInt8Ty(ctx);
    IntTy = llvm::Type::getInt64Ty(ctx);
    SubFuncTy = llvm::FunctionType::get(VoidTy, false);

    NodePtrTy = llvm::PointerType::getUnqual(IntTy);
    StackNodeTy = llvm::StructType::create(NodePtrTy);
    StackPtrTy = llvm::PointerType::getUnqual(StackNodeTy);

    TagTy = llvm::Type::getInt8Ty(ctx);

    std::vector<llvm::Type *> Values = {
    TagTy, 
    llvm::PointerType::getUnqual(IntTy),
    llvm::PointerType::getUnqual(DoubleTy),
    llvm::PointerType::getUnqual(CharTy)
    };

    ConstantNodeTy = llvm::StructType::create(Values, "constant_node");
    AppNodeTy = llvm::StructType::create({TagTy, NodePtrTy, NodePtrTy}, "app_node");

    // func_node will consist of tag 
    // and pointer to the function where parameter substitution will be made
    // that's why it will be bitcasted

    FuncNodeTy = llvm::StructType::create(
        {
        TagTy, 
        IntTy,
        llvm::PointerType::getUnqual(SubFuncTy)
        }, "func_node"
    );
    IndNodeTy = llvm::StructType::create({TagTy, NodePtrTy});
}

void GMachineState::InitGlobal() {
    stack_top = new llvm::GlobalVariable(
        module,            
        StackPtrTy,             
        false,                 
        llvm::GlobalValue::ExternalLinkage, 
        nullptr,            
        "stack_top"        
    );
}

void GMachineState::DefineFunctions() {
    std::cout << "#1.1\n";
    {
        llvm::FunctionType* mainFunctionType = llvm::FunctionType::get(builder.getInt32Ty(), false);
        Main = llvm::Function::Create(mainFunctionType, llvm::Function::ExternalLinkage, "main", module);
    }
    
    std::cout << "#1.3\n";
    //MAIN START
    {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", Main);
        builder.SetInsertPoint(entry);
        std::cout << "#1\n";
        builder.CreateLoad(StackNodeTy, builder.CreateAlloca(StackNodeTy, 1000000), "init_stack");
        std::cout << "#2\n";
        GMachineState::Context c(this, {}, 0);
        std::cout << "#3\n";
        GCode::PushGlobal("PROG").adapt(&c);
        std::cout << "#4\n";
        GCode::Eval().Call(this);
        std::cout << "#5\n";

        auto PrintF = 
        module.getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(ctx), 
        llvm::PointerType::get(llvm::Type::getInt8Ty(ctx), 
        0), true /* this is var arg func type*/) 
        );
        std::cout << "#6\n";
        llvm::Constant* formatStr = builder.CreateGlobalStringPtr("%ld\n", "formatStr");
        std::cout << "#7\n";
        builder.CreateCall(PrintF, {formatStr, LoadFromConstantNode(this, LoadStackNode(this, stack_top), _INT)});
        std::cout << "#8\n";
        builder.CreateRet(builder.getInt32(0));
        std::cout << "#9\n";
    }
    //MAIN END
    std::cout << "#1.4\n";
    for (auto f : sub_funcs) {
        f.second->compile_definition();
    }
    for (auto f : sub_funcs) {
        f.second->compile_body();
    }
    std::cout << "#1.5\n";
}