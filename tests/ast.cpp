#include "ast.h"
#include <functional>
#include <gtest/gtest.h>

#include <memory>
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
        new Wrap(
            [](const LambdaNode& v) {
                ASSERT_EQ(v.GetBonded().size(), 1);
                ASSERT_EQ(v.GetBonded()[0].lock()->GetName(), "x");
            }
        ),
        new Wrap(&VarNode::GetName, "x")
    };
    format = "#0(V1)";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

std::function VarComp = [](const std::weak_ptr<VarNode> lhs, const std::weak_ptr<VarNode> rhs) {
    return lhs.lock()->GetName() < rhs.lock()->GetName();
};

TEST_F(ast, x_plus_y) {
    make_sample("+ x y"); 
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            [](const AppNode& v) {
                auto vec = v.GetFreeVar();
                std::sort(vec.begin(), vec.end(), VarComp);
                ASSERT_EQ(vec[0].lock()->GetName(), "x");
                ASSERT_EQ(vec[1].lock()->GetName(), "y");
            }
        ),
        new Wrap<AppNode>(),
        new Wrap<AddNode>(),
        new Wrap(&VarNode::GetName, "x"),
        new Wrap(&VarNode::GetName, "y")
    };
    format = "@0(@1(*2)(V3))(V4)";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, plus_func) {
    make_sample("@x.@y.% x y");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            [](const LambdaNode& v) {
                auto vec = v.GetBonded();
                ASSERT_EQ(vec.size(), 1);
                ASSERT_EQ(vec[0].lock()->GetName(), "x");
            }
        ),
        new Wrap(
            [](const LambdaNode& v){
                auto free = v.GetFreeVar();
                auto bonded = v.GetBonded();
                ASSERT_EQ(free.size(), 1);
                ASSERT_EQ(bonded.size(), 1);
                ASSERT_EQ(free[0].lock()->GetName(), "x");
                ASSERT_EQ(bonded[0].lock()->GetName(), "y");
            }
        ),
        new Wrap<RemNode>()
    };
    format = "#0(#1(@(@(*2)(V))(V)))";;
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, double_combinator) {
    make_sample("(@x.x x) @x.x x");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap(
            [](const LambdaNode& v) {
                auto vec = v.GetBonded();
                ASSERT_EQ(vec.size(), 2);
            }
        ),
        new Wrap(
            [](const LambdaNode& v){
                auto vec = v.GetBonded();
                ASSERT_EQ(vec.size(), 2);
            }
        )  ,
        new Wrap
        (&AppNode::GetDepth, 3)
    };
    format = "@2(#0(@(V)(V)))(#1(@(V)(V)))";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}


TEST_F(ast, double_combinator_if_no_parenthesis) {
    make_sample("@x.x x @x.x x");
    ASSERT_EQ(yyparse(root), 0);
    types = {
        new Wrap
        ([](const LambdaNode& v) {
            ASSERT_EQ(v.GetFreeVar().size(), 0);
        },
        &AppNode::GetDepth, 4)
    };
    format = "#0(@(@(V)(V))(#(@(V)(V))))";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}

TEST_F(ast, y_combinator) {
    make_sample("@f.(@x.f(x x))(@x.f(x x))");
    ASSERT_EQ(yyparse(root), 0);
    format = "#(@(#(@(V)(@(V)(V))))(#(@(V)(@(V)(V)))))";
    real_root = std::make_shared<Tree>(format, start, types);
    real_root->check(real_root, root);
}
