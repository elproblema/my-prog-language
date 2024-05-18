#pragma once

#include <gtest/gtest.h>

#include <fstream>

#include <parser.hpp>
#include <scanner.hpp>
#include "help.h"
#include <ast/SC.h>

// ####################### FIXTURES ##########################

//fixtures below are used for debuging convinience

static int failed = 0; //NOLINT
static int passed = 0; //NOLINT

struct ast : public testing::Test {
  public:
    size_t start = 0;
    std::shared_ptr<Node> root;
    std::shared_ptr<Tree> real_root;
    std::vector<Base*> types;
    std::string format = "";

    void SetUp() {
        freopen("debug", "w", stderr);
        set_map();
        yydebug = 1;
        yy_flex_debug = 1;
        extern int yycolumn; 
        extern int yylineno; 
        extern int yy_next_column;
        yycolumn = 1;
        yylineno = 1;
        yy_next_column = 1;
    }

    void TearDown() {
        if (testing::UnitTest::GetInstance()->failed_test_case_count() - failed) {
            std::cerr.flush();
            std::cout << "\n\n#########PARSERS DEBUG INFO#########\n\n";
            std::ifstream in("debug");
            std::cout << in.rdbuf();
            std::cout.flush();
        }
        for (auto ptr : types) delete ptr;
        failed = testing::UnitTest::GetInstance()->failed_test_case_count();
        passed = testing::UnitTest::GetInstance()->successful_test_case_count();
    }
};

struct rebuild : ast {

    void SetUp() {
        ast::SetUp();
        extern SC_container all_functions;
        all_functions.clear();
        yy_flex_debug = 0;
        yydebug = 0;
    }
};