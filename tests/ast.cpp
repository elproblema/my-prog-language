#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <parser.hpp>
#include <scanner.h>

void make_sample(const char* s) {
    FILE* f = fopen("input", "w");
    fprintf(f, "%s", s);
    fclose(f);
    freopen("input", "r", stdin);
}

TEST(ast, variable) {
    make_sample("x");
    Node* root;
    ASSERT_EQ(yyparse(root), 0);
    ASSERT_NO_THROW(dynamic_cast<VarNode&>(*root));
}

TEST(ast, constant) {
    make_sample("5");
    Node* root;
    ASSERT_EQ(yyparse(root), 0);
    ASSERT_NO_THROW(ConstNode& n = dynamic_cast<ConstNode&>(*root););
    //ASSERT_EQ(n)
}