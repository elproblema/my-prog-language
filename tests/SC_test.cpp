#include <gtest/gtest.h>

#include <ast.h>
#include <SC.h>

#include <functional>
#include <memory>

#include <parser.hpp>
#include <scanner.hpp>

#include "help.h"
#include "fixtures.h"

extern SC_container all_functions;

TEST_F(rebuild, plus_func) {
    make_sample("(@x.@y.+ x y) 4 3");
    ASSERT_EQ(yyparse(root), 0);
    RebuildAst(root);
    ASSERT_EQ(all_functions.size(), 1);
    ASSERT_EQ(all_functions[0]->GetFuncName(), "PROG");

    format = "S0(@(@(*1)(C2))(C3))";
    types = {
        new Wrap([](const SuperCombinator& v) {
            ASSERT_EQ(v.GetFuncName(), "PROG");
        }),
        new Wrap<AddNode>(),
        new Wrap(&ConstNode::GetLL, 4),
        new Wrap(&ConstNode::GetLL, 3)
    };
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, all_functions.back());
}