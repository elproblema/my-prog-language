#pragma once

#include <gtest/gtest.h>
#include <parser.hpp>
#include <scanner.hpp>
#include "tree.h"

// ####################### FIXTURES ##########################

struct ast_nondebug : public testing::Test {
    std::shared_ptr<Node> root;

    void SetUp() {
        yydebug = 0;
        yy_flex_debug = 0;
        set_map();
    }

    void TearDown() {
    }
};

//all fixtures below are used for debuging convinience

struct ast_alldebug : public testing::Test {
    std::shared_ptr<Node> root;

    void SetUp() {
        yydebug = 1;
        yy_flex_debug = 1;
        set_map();
    }

    void TearDown() {
    }
};

struct ast_flexdebug : public testing::Test {
    std::shared_ptr<Node> root;

    void SetUp() {
        yydebug = 0;
        yy_flex_debug = 1;
        set_map();
    }

    void TearDown() {
    }
};

struct ast_bisondebug : public testing::Test {
    std::shared_ptr<Node> root;

    void SetUp() {
        yydebug = 1;
        yy_flex_debug = 0;
        set_map();
    }

    void TearDown() {
    }
};