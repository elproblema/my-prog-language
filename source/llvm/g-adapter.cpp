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


GMachineState::GMachineState(): 
builder(ctx), module("main", ctx)
{
    DefineTypes();
    DefineFunctions();
    InitGlobal();
    PrepareToExecute();
}

void GMachineState::DefineTypes() {
    VoidTy = llvm::Type::getVoidTy(ctx);
    DoubleTy = llvm::Type::getDoubleTy(ctx);
    CharTy = llvm::Type::getInt8Ty(ctx);
    IntTy = llvm::Type::getInt64Ty(ctx);

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
        llvm::PointerType::getUnqual(IntTy)
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
    // UNWIND START
    {
        auto FuncTy = llvm::FunctionType::get(VoidTy, false);
        UnwindFunc = llvm::Function::Create(FuncTy, llvm::Function::ExternalLinkage, "unwind", module);
        auto entry = llvm::BasicBlock::Create(ctx, "entry", UnwindFunc);

        builder.SetInsertPoint(entry);
        auto AppBB = llvm::BasicBlock::Create(ctx, "app_node");
        auto FuncBB = llvm::BasicBlock::Create(ctx, "func_node");
        auto MergeBB = llvm::BasicBlock::Create(ctx, "merge");

        auto SwitchValue = LoadTag(this, stack_top);
        auto Switch = builder.CreateSwitch(SwitchValue, MergeBB, 2);
        Switch->addCase(ConstantChar(this, APP), AppBB);
        Switch->addCase(ConstantChar(this, FUNC), FuncBB);

        builder.SetInsertPoint(AppBB);
        auto lhs = LoadFromAppNode(this, LoadStackNode(this, stack_top), 0);
        builder.CreateStore(PointerAdd(this, stack_top, 1), stack_top);
        builder.CreateStore(lhs, LoadStackNode(this, stack_top));
        builder.CreateBr(MergeBB);

        builder.SetInsertPoint(FuncBB);
        auto val = builder.CreateStructGEP(FuncNodeTy, LoadStackNode(this, stack_top), 1);
        builder.CreateCall(UnpackFunc, val);
        builder.CreateBr(MergeBB);

        builder.SetInsertPoint(MergeBB);
        builder.CreateRetVoid();
    }

    //UNWIND END
    //UNPACK START
    {
        auto FuncTy = llvm::FunctionType::get(VoidTy, IntTy, false);
        UnpackFunc = llvm::Function::Create(FuncTy, llvm::Function::ExternalLinkage, "unpack", module);
        auto entry = llvm::BasicBlock::Create(ctx, "entry", UnpackFunc);
        auto ret_block = llvm::BasicBlock::Create(ctx, "ret_label");
        auto loop_block = llvm::BasicBlock::Create(ctx, "loop_label");

        builder.SetInsertPoint(entry);
        auto ptr_to_ind = builder.CreateAlloca(IntTy, nullptr, "ind");
        builder.CreateStore(ConstantInt(this, 0), ptr_to_ind);
        builder.CreateBr(loop_block);

        builder.SetInsertPoint(ret_block);
        builder.CreateRetVoid();

        builder.SetInsertPoint(loop_block);
        auto ptr_to_node_on_ind = builder.CreatePtrAdd(stack_top, builder.CreateLoad(IntTy, ptr_to_ind));
        auto ptr_to_node_on_ind_minus_1 = PointerAdd(this, ptr_to_node_on_ind, -1);
        StoreStackNode(this, LoadFromAppNode(this, ptr_to_node_on_ind_minus_1, 1), ptr_to_node_on_ind);
        builder.CreateStore(builder.CreateAdd(builder.CreateLoad(IntTy, ptr_to_ind), ConstantInt(this, 1)), ptr_to_ind);
        auto cond = builder.CreateICmpSLT(builder.CreateLoad(IntTy, ptr_to_ind), UnwindFunc->arg_begin());
        builder.CreateCondBr(cond, loop_block, ret_block);
    }
    //UNPACK END
}