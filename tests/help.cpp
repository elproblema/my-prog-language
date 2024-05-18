#include <gtest/gtest.h>

#include <cassert>
#include <map>
#include <functional>

#include <parser.hpp>
#include <scanner.hpp>
#include <ast/ast.h>
#include <ast/SC.h>
#include "help.h"

std::map<char, std::function<void(std::shared_ptr<Tree>, std::shared_ptr<const Node>)>> char_map_func; //NOLINT

void make_sample(const char* s) { //NOLINT
    FILE* f = fopen("input", "w");
    fprintf(f, "%s", s);
    fclose(f);
    freopen("input", "r", stdin);
}

void set_map() { //NOLINT
    char_map_func = {
        {'@', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 2); 
            vertex->template_->check(*opp_vertex);
            auto func = vertex->children[0];
            auto arg = vertex->children[1];
            auto func_opp = opp_vertex->GetFunc();
            auto arg_opp = opp_vertex->GetArg();
            func->check(func, func_opp); 
            arg->check(arg, arg_opp);
        }},
        {'V', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 0); 
            vertex->template_->check(*opp_vertex);

        }},
        {'*', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 0); 
            vertex->template_->check(*opp_vertex);

        }},
        {'C', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 0); 
            vertex->template_->check(*opp_vertex);

        }},
        {'#', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 1); 
            vertex->template_->check(*opp_vertex);
            auto body = vertex->children[0];
            auto body_opp = opp_vertex->GetBody();
            body->check(body, body_opp);
        }},
        {'S', [](std::shared_ptr<Tree> vertex, std::shared_ptr<const Node> opp_vertex) -> void { 
            ASSERT_EQ(vertex->children.size(), 1); 
            vertex->template_->check(*opp_vertex);
            auto body = vertex->children[0];
            auto body_opp = opp_vertex->GetBody();
            body->check(body, body_opp);
        }}
    };
}

void unset_map() { //NOLINT
    char_map_func = {};
}