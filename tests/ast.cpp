#include "ast.h"
#include <gtest/gtest.h>

#include <parser.hpp>
#include <scanner.hpp>
#include "help.h"
#include "fixtures.h"


//######################### TESTS ############################

TEST_F(ast, error) {
    make_sample("@@");
    ASSERT_EQ(yyparse(root), 1);
}

TEST_F(ast, variable) {
    make_sample("x");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            [](const VarNode& v) {
                ASSERT_EQ(v.GetFreeVar().size(), 1);
            },
            &VarNode::GetName, "x",
            &VarNode::GetDepth, 0
        )
    };
    format = "V0";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, constant_int) {
    make_sample("5");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            &ConstNode::GetLL, 5,
            &ConstNode::GetDepth, 0,
            [](const ConstNode& v) {
                ASSERT_EQ(v.GetFreeVar().size(), 0);
            }
        )
    };
    format = "V0";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, constant_float) {
    make_sample(".6");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            &ConstNode::GetLD, .6,
            &ConstNode::GetDepth, 0,
            [](const ConstNode& v) {
                ASSERT_EQ(v.GetFreeVar().size(), 0);
            }
        )
    };
    format = "V0";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, constant_char) {
    make_sample("\'\\\'"); // this is just '\' string
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            &ConstNode::GetChar, '\\',
            &ConstNode::GetDepth, 0,
            [](const ConstNode& v) {
                ASSERT_EQ(v.GetFreeVar().size(), 0);
            }
        )
    };
    format = "V0";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, free_var_check) {
    make_sample("x y z");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            &AppNode::GetDepth, 2,
            [](const AppNode& v) {
                ASSERT_EQ(v.GetFreeVar().size(), 3);
            }
        )
    };
    format = "@0(@(V)(V))(V)";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, plus) {
    make_sample("+");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap<AddNode>()
    };
    format = "*0";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, identity) {
    make_sample("@x.x");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap<LambdaNode>(),
        new Wrap<VarNode>()
    };
    format = "#0(V1)";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}
/*
TEST_F(ast, x_plus_y) {
    make_sample("+ x y");
    std::string real_tree = "@(@(*)(V))(V)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 2);
    ASSERT_EQ(root->GetFreeVar().size(), 2);
}

TEST_F(ast, plus_func) {
    make_sample("@x.@y.+ x y");
    std::string real_tree = "#(#(@(@(*)(V))(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 4);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast, const_plus) {
    make_sample("+ 3 5");
    std::string real_tree = "@(@(*)(C))(C)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 2);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast, double_combinator) {
    make_sample("(@x.x x) @x.x x");
    std::string real_tree = "@(#(@(V)(V)))(#(@(V)(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 3);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast, double_combinator_if_no_parenthesis) {
    make_sample("@x.x x @x.x x");
    std::string real_tree = "#(@(@(V)(V))(#(@(V)(V))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 4);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast, y_combinator) {
    make_sample("@f.(@x.f(x x))(@x.f(x x))");
    std::string real_tree = "#(@(#(@(V)(@(V)(V))))(#(@(V)(@(V)(V)))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 5);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast, compile) {
    types = {new Wrap(&ConstNode::GetLL, 5), new Wrap<LambdaNode>()};
}*/