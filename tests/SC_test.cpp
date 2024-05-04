#include <gtest/gtest.h>

#include <ast.h>
#include <SC.h>

#include <functional>
#include <memory>

#include <parser.hpp>
#include <scanner.hpp>

#include "help.h"
#include "fixtures.h"
#include "printer.cpp"

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

TEST_F(rebuild, inf_list) {
    make_sample("@f.@x.CONS x (f(+ x 1))");
    ASSERT_EQ(yyparse(root), 0);
    RebuildAst(root);
    ASSERT_EQ(all_functions.size(), 2);

    format = "S0(S1(@(@(*2)(V3))(@(V4)(@(@(*5)(V6))(C7)))))";
    types = {
        new Wrap(&SuperCombinator::GetFuncName, "PROG"),
        new Wrap(&SuperCombinator::GetFuncName, "1:4"),
        new Wrap<ConsNode>(),
        new Wrap(&VarNode::GetName, "x"),
        new Wrap(&VarNode::GetName, "f"),
        new Wrap<AddNode>(),
        new Wrap(&VarNode::GetName, "x"),
        new Wrap(&ConstNode::GetLL, 1)
    };
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, all_functions.back());
}

TEST_F(rebuild, Y_combinator) {
    make_sample("@f.(@x.f(x x))(@y.f(y y))");
    ASSERT_EQ(yyparse(root), 0);
    RebuildAst(root);
    ASSERT_EQ(all_functions.size(), 4);
    
    format = "S0(S1(@(@(S2(@(V4)(@(V5)(V5))))(V4))(@(S3(@(V4)(@(V6)(V6))))(V4))))";
    types = {
        new Wrap(&SuperCombinator::GetFuncName, "PROG"),
        new Wrap(
            [](const SuperCombinator& v) {
                ASSERT_EQ(v.GetSomeVars().size(), 1);
                ASSERT_EQ(v.GetSomeVars()[0].lock()->GetName(), "f");
            },
            &SuperCombinator::GetFuncName, "1:1"
        ),
        new Wrap(
            [](const SuperCombinator& v) {
                ASSERT_EQ(v.GetSomeVars().size(), 2);
                ASSERT_EQ(v.GetSomeVars()[0].lock()->GetName(), "f");
                ASSERT_EQ(v.GetSomeVars()[1].lock()->GetName(), "x");
            },
            &SuperCombinator::GetFuncName, "1:4"
        ),
        new Wrap(
            [](const SuperCombinator& v) {
                ASSERT_EQ(v.GetSomeVars().size(), 2);
                ASSERT_EQ(v.GetSomeVars()[0].lock()->GetName(), "f");
                ASSERT_EQ(v.GetSomeVars()[1].lock()->GetName(), "y");
            },
            &SuperCombinator::GetFuncName, "1:15"
        ),
        new Wrap(&VarNode::GetName, "f"),
        new Wrap(&VarNode::GetName, "x"),
        new Wrap(&VarNode::GetName, "y")
    };
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, all_functions.back());
}