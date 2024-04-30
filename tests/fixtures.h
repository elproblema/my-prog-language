#pragma once

#include <gtest/gtest.h>

#include <fstream>

#include <parser.hpp>
#include <scanner.hpp>
#include "help.h"

// ####################### FIXTURES ##########################

//fixtures below are used for debuging convinience

struct ast : public testing::Test {
  private:
    bool setup = false; 
    bool teardown = false;

  public:
    size_t start = 0;
    std::shared_ptr<Node> root;
    std::shared_ptr<Tree> real_root;
    std::vector<Base*> types;
    std::string format;

    void SetUp() {
        freopen("debug", "w", stderr);
        set_map();
        yydebug = 1;
        yy_flex_debug = 1;
    }

    void TearDown() {
        if (testing::UnitTest::GetInstance()->Failed()) {
            std::ifstream in("debug");
            std::cerr << in.rdbuf();
        }
        for (auto ptr : types) delete ptr;
    }
};