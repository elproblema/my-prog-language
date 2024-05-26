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

extern SC_container all_functions;

GMachineState::GMachineState(): 
builder(ctx), module("main", ctx)
{
    DefineTypes();
    InitGlobal();
    DefineFunctions();
}

void GMachineState::DefineTypes() {
    VoidTy = llvm::Type::getVoidTy(ctx);
    DoubleTy = llvm::Type::getDoubleTy(ctx);
    CharTy = llvm::Type::getInt8Ty(ctx);
    IntTy = llvm::Type::getInt64Ty(ctx);
    SubFuncTy = llvm::FunctionType::get(VoidTy, false);

    NodePtrTy = llvm::PointerType::getUnqual(IntTy);
    StackNodeTy = llvm::StructType::create(NodePtrTy, "stack_node");
    StackPtrTy = llvm::PointerType::getUnqual(StackNodeTy);

    TagTy = llvm::Type::getInt8Ty(ctx);

    std::vector<llvm::Type *> Values = {
    TagTy, 
    IntTy,
    DoubleTy,
    CharTy
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
        SubFuncTy->getPointerTo()
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
        llvm::ConstantPointerNull::get(StackPtrTy),            
        "stack_top"        
    );
}

void GMachineState::DefineFunctions() {
    for (auto f : all_functions) {
        (new FuncSubstitution(this, f))->compile_definition();
    }
    {
        llvm::FunctionType* mainFunctionType = llvm::FunctionType::get(builder.getInt32Ty(), false);
        Main = llvm::Function::Create(mainFunctionType, llvm::Function::ExternalLinkage, "main", module);
    }

    GCode::Unwind::Init(this);
    GCode::Eval::Init(this);
    
    //MAIN START
    {
        llvm::BasicBlock* entry = llvm::BasicBlock::Create(ctx, "entry", Main);
        builder.SetInsertPoint(entry);
        DebugPrint(this, "#1\n");
        StoreStackTop(this, builder.CreateAlloca(StackNodeTy, ConstantInt(this, 1000000)));
        DebugPrint(this, "#2\n");
        GMachineState::Context c(this, {}, 0);
        GCode::PushGlobal("PROG").adapt(&c);
        DebugPrint(this, "#3\n");
        GCode::Eval().Call(this);
        DebugPrintLL(this, "const val out of eval call", LoadFromConstantNode(this, LoadStackNode(this, LoadStackTop(this)), _INT));


        auto PrintF = 
        module.getOrInsertFunction(
        "printf",
        llvm::FunctionType::get(llvm::IntegerType::getInt32Ty(ctx), 
        llvm::Type::getInt8Ty(ctx)->getPointerTo(), true /* this is var arg func type*/) 
        );
        llvm::Constant* formatStr = builder.CreateGlobalStringPtr("%lld\n", "formatStr");
        DebugPrint(this, "before print\n");
        DebugPrintLL(this, "const val", LoadStackTop(this));
        builder.CreateCall(PrintF, {formatStr, LoadFromConstantNode(this, LoadStackNode(this, LoadStackTop(this)), _INT)});
        builder.CreateRet(builder.getInt32(0));
    }
    //MAIN END
    for (auto el : sub_funcs) {
        el.second->compile_body();
    }
}