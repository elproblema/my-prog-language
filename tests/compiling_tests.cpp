#include "fixtures.h"
#include <ast/SC.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Verifier.h>

#include <llvm/g-adapter.h>
#include <llvm/g-commands.h>
#include <llvm/llvm-shortcuts.h>
#include <llvm/g-sub.h>

extern SC_container all_functions;

TEST_F(compile, number) {
    make_sample("3");
    ASSERT_EQ(yyparse(root), 0);
    RebuildAst(root);
    ASSERT_EQ(all_functions.size(), 1);
    GMachineState st;
    llvm::verifyFunction(*st.Main);
    llvm::verifyModule(st.module, &llvm::errs());
    st.module.print(llvm::outs(), nullptr);
}