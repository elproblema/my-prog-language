#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <parser.hpp>
#include <scanner.hpp>

// ################## HELPING STRUCTURES #####################

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
        }}
    };
}

void unset_map() {
    char_map_func = {};
}

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
