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


GMachineState::GMachineState(): 
builder(ctx), module("main", ctx), 
stack_node(llvm::StructType::create(ctx, "stack")),
stack_ptr(llvm::PointerType::getUnqual(stack_node))
{
    DefineTypes();
    DefineFunctions();
    InitGlobal();
}

void GMachineState::DefineTypes() {
    llvm::Type *DoubleTy = llvm::Type::getDoubleTy(ctx);
    llvm::Type *CharTy = llvm::Type::getInt8Ty(ctx);
    llvm::Type *IntTy = llvm::Type::getInt64Ty(ctx);

    node_ptr = llvm::PointerType::get(llvm::Type::getInt32Ty(ctx), 0);
    stack_node = llvm::StructType::create(node_ptr);
    stack_ptr = llvm::PointerType::get(stack_node, 0);

    tag = llvm::Type::getInt8Ty(ctx);

    std::vector<llvm::Type *> Values = {
    tag, 
    llvm::PointerType::getUnqual(DoubleTy),
    llvm::PointerType::getUnqual(CharTy),
    llvm::PointerType::getUnqual(IntTy)
    };

    constant_node = llvm::StructType::create(Values, "constant_node");
    app_node = llvm::StructType::create({tag, node_ptr, node_ptr}, "app_node");

    // func_node will consist of tag 
    // and pointer to the function where parameter substitution will be made
    // that's why it will be bitcasted

    func_node = llvm::StructType::create(
        {tag, 
        llvm::PointerType::get(llvm::Type::getInt32Ty(ctx), 0)
        }, "func_node"
    );
    ind_node = llvm::StructType::create({tag, node_ptr});
}

void GMachineState::InitGlobal() {
    stack_top = new llvm::GlobalVariable(
        module,            
        stack_ptr,             
        false,                 
        llvm::GlobalValue::ExternalLinkage, 
        llvm::ConstantPointerNull::get(stack_ptr),            
        "stack_top"        
    );
}
