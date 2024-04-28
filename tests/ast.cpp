#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <parser.hpp>
#include <scanner.hpp>
#include "tree.h"
#include "fixtures.h"


//######################### TESTS ############################

TEST_F(ast_nondebug, error) {
    make_sample("@@");
    ASSERT_EQ(yyparse(root), 1);
}

TEST_F(ast_nondebug, variable) {
    make_sample("x");
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<VarNode> new_root = std::dynamic_pointer_cast<VarNode>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(new_root->GetName(), "x");
    ASSERT_EQ(new_root->GetDepth(), 0);
    ASSERT_EQ(new_root->GetFreeVar().size(), 1);
}

TEST_F(ast_nondebug, constant_int) {
    make_sample("5");
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<ConstNode> new_root = std::dynamic_pointer_cast<ConstNode>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(5LL, std::get<0>(new_root->GetValue()));
    ASSERT_EQ(new_root->GetDepth(), 0);
    ASSERT_EQ(new_root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, constant_float) {
    make_sample(".6");
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<ConstNode> new_root = std::dynamic_pointer_cast<ConstNode>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(0.6L, std::get<1>(new_root->GetValue()));
    ASSERT_EQ(new_root->GetDepth(), 0);
    ASSERT_EQ(new_root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, constant_char) {
    make_sample("\'\\\'"); // this is just '\' string
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<ConstNode> new_root = std::dynamic_pointer_cast<ConstNode>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ('\\', std::get<2>(new_root->GetValue()));
    ASSERT_EQ(new_root->GetDepth(), 0);
    ASSERT_EQ(new_root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, free_var_check) {
    make_sample("x y z");
    ASSERT_EQ(yyparse(root), 0);
    ASSERT_EQ(root->GetFreeVar().size(), 3);
    ASSERT_EQ(root->GetDepth(), 2);
}

TEST_F(ast_nondebug, plus) {
    make_sample("+");
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<AddNode> new_root = std::dynamic_pointer_cast<AddNode>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(new_root->GetDepth(), 0);
    ASSERT_EQ(new_root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, identity) {
    make_sample("@x.x");
    std::string real_tree = "#(V)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 1);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, x_plus_y) {
    make_sample("+ x y");
    std::string real_tree = "@(@(*)(V))(V)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 2);
    ASSERT_EQ(root->GetFreeVar().size(), 2);
}

TEST_F(ast_nondebug, plus_func) {
    make_sample("@x.@y.+ x y");
    std::string real_tree = "#(#(@(@(*)(V))(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 4);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, const_plus) {
    make_sample("+ 3 5");
    std::string real_tree = "@(@(*)(C))(C)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 2);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, double_combinator) {
    make_sample("(@x.x x) @x.x x");
    std::string real_tree = "@(#(@(V)(V)))(#(@(V)(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 3);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, double_combinator_if_no_parenthesis) {
    make_sample("@x.x x @x.x x");
    std::string real_tree = "#(@(@(V)(V))(#(@(V)(V))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 4);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}

TEST_F(ast_nondebug, y_combinator) {
    make_sample("@f.(@x.f(x x))(@x.f(x x))");
    std::string real_tree = "#(@(#(@(V)(@(V)(V))))(#(@(V)(@(V)(V)))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    std::shared_ptr<Tree> real_root = std::make_shared<Tree>(real_tree, start);
    real_root->check(real_root, root);
    ASSERT_EQ(root->GetDepth(), 5);
    ASSERT_EQ(root->GetFreeVar().size(), 0);
}
