#pragma once

#include <gtest/gtest.h>
#include <cassert>
#include <map>
#include <functional>
#include <iostream>
#include <sstream>
#include <parser.hpp>
#include <scanner.hpp>
#include <ast.h>
#include <SC.h>

// ###################### HELPING STRUCTURES #########################

struct Tree;
extern int yy_flex_debug;

std::map<char, std::function<void(std::shared_ptr<Tree>, std::shared_ptr<const Node>)>> char_map_func;

struct Tree {
    std::function<void(std::shared_ptr<Tree>, std::shared_ptr<const Node>)> check;
    std::vector<std::shared_ptr<Tree>> children;

    Tree(std::string& format, size_t& start) {
        size_t n = format.size();
        assert(n > start);
        char c = format[start];
        assert(c == '@' || c == 'V' || c == '*' || c == 'C' || c == '#');
        check = char_map_func[c];
        ++start;
        while (format[start] == '(' && start < n) {
            ++start;
            children.push_back(std::make_shared<Tree>(format, start));
        }
        if (format[start] == ')') { 
            ++start;
            return;
        }
    }
};


void make_sample(const char* s) {
    FILE* f = fopen("input", "w");
    fprintf(f, "%s", s);
    fclose(f);
    freopen("input", "r", stdin);
}

void set_map() {
    char_map_func = {
        {'@', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            std::shared_ptr<const AppNode> opp_cast = std::dynamic_pointer_cast<const AppNode>(opp_vertex);
            ASSERT_NE(opp_cast, nullptr);
            ASSERT_EQ(vertex->children.size(), 2); 
            std::shared_ptr<Tree> func = vertex->children[0];
            std::shared_ptr<Tree> arg = vertex->children[1];
            std::shared_ptr<const Node> func_opp = opp_cast->GetFunc();
            std::shared_ptr<const Node> arg_opp = opp_cast->GetArg();
            func->check(func, func_opp); 
            arg->check(arg, arg_opp);
        }},
        {'V', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_NE(std::dynamic_pointer_cast<const VarNode>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 

        }},
        {'*', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_NE(std::dynamic_pointer_cast<const BIFNode>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 
        }},
        {'C', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_NE(std::dynamic_pointer_cast<const ConstNode>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 
        }},
        {'#', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            std::shared_ptr<const LambdaNode> opp_cast = std::dynamic_pointer_cast<const LambdaNode>(opp_vertex);
            ASSERT_NE(opp_cast, nullptr); 
            ASSERT_EQ(vertex->children.size(), 1); 
            std::shared_ptr<Tree> body = vertex->children[0];
            std::shared_ptr<const Node> body_opp = opp_cast->GetBody();
            body->check(body, body_opp); 
        }}/*,
        {'SC', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            std::shared_ptr<const SuperCombinator> opp_cast = std::dynamic_pointer_cast<const SuperCombinator>(opp_vertex);
            ASSERT_NE(opp_cast, nullptr); 
            ASSERT_EQ(vertex->children.size(), 1); 
            std::shared_ptr<Tree> body = vertex->children[0];
            std::shared_ptr<const Node> body_opp = opp_cast->GetBody();
            body->check(body, body_opp); 
        }}*/
    };
}

void unset_map() {
    char_map_func = {};
}