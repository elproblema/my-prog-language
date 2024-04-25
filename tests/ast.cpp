#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <parser.hpp>
#include <scanner.hpp>

// ################## HELPING STRUCTURES #####################

struct Tree;
extern int yy_flex_debug;

std::map<char, std::function<void(Tree*, const Node*)>> char_map_func;

struct Tree {
    std::function<void(Tree*, const Node*)> check;
    std::vector<Tree*> children;

    Tree(std::string& format, size_t& start) {
        size_t n = format.size();
        assert(n > start);
        char c = format[start];
        assert(c == '@' || c == 'V' || c == '*' || c == 'C' || c == '#');
        check = char_map_func[c];
        ++start;
        while (format[start] == '(' && start < n) {
            ++start;
            children.push_back(new Tree(format, start));
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
        {'@', [](Tree* vertex, const Node* opp_vertex) -> void { 
            const AppNode* opp_cast = dynamic_cast<const AppNode*>(opp_vertex);
            ASSERT_NE(opp_cast, nullptr);
            ASSERT_EQ(vertex->children.size(), 2); 
            Tree* func = vertex->children[0];
            Tree* arg = vertex->children[1];
            const Node* func_opp = opp_cast->GetFunc();
            const Node* arg_opp = opp_cast->GetArg();
            func->check(func, func_opp); 
            arg->check(arg, arg_opp);
        }},
        {'V', [](Tree* vertex, const Node* opp_vertex) -> void { 
            ASSERT_NE(dynamic_cast<const VarNode*>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 

        }},
        {'*', [](Tree* vertex, const Node* opp_vertex) -> void { 
            ASSERT_NE(dynamic_cast<const BIFNode*>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 
        }},
        {'C', [](Tree* vertex, const Node* opp_vertex) -> void { 
            ASSERT_NE(dynamic_cast<const ConstNode*>(opp_vertex), nullptr); 
            ASSERT_EQ(vertex->children.size(), 0); 
        }},
        {'#', [](Tree* vertex, const Node* opp_vertex) -> void { 
            const LambdaNode* opp_cast = dynamic_cast<const LambdaNode*>(opp_vertex);
            ASSERT_NE(opp_cast, nullptr); 
            ASSERT_EQ(vertex->children.size(), 1); 
            Tree* body = vertex->children[0];
            const Node* body_opp = opp_cast->GetBody();
            body->check(body, body_opp); 
        }}
    };
}

void unset_map() {
    char_map_func = {};
}

// ################## FIXTURES #####################

struct ast_alldebug : public testing::Test {
    Node* root;

    void SetUp() {
        yydebug = 1;
        yy_flex_debug = 1;
        set_map();
    }

    void TearDown() {
    }
};

struct ast_nondebug : public testing::Test {
    Node* root;

    void SetUp() {
        yydebug = 0;
        yy_flex_debug = 0;
        set_map();
    }

    void TearDown() {
    }
};

struct ast_flexdebug : public testing::Test {
    Node* root;

    void SetUp() {
        yydebug = 0;
        yy_flex_debug = 1;
        set_map();
    }

    void TearDown() {
    }
};

struct ast_bisondebug : public testing::Test {
    Node* root;

    void SetUp() {
        yydebug = 1;
        yy_flex_debug = 0;
        set_map();
    }

    void TearDown() {
    }
};


// ################## TESTS #####################



TEST_F(ast_nondebug, error) {
    make_sample("@@");
    ASSERT_EQ(yyparse(root), 1);
}

TEST_F(ast_nondebug, variable) {
    make_sample("x");
    ASSERT_EQ(yyparse(root), 0);
    VarNode* new_root = dynamic_cast<VarNode*>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(new_root->GetName(), "x");
}

TEST_F(ast_nondebug, constant) {
    make_sample("5");
    ASSERT_EQ(yyparse(root), 0);
    ConstNode* new_root = dynamic_cast<ConstNode*>(root);
    ASSERT_NE(new_root, nullptr);
    ASSERT_EQ(5LL, std::get<0>(new_root->GetValue()));
}

TEST_F(ast_nondebug, plus) {
    make_sample("+");
    ASSERT_EQ(yyparse(root), 0);
    AddNode* new_root = dynamic_cast<AddNode*>(root);
    ASSERT_NE(new_root, nullptr);
}

TEST_F(ast_nondebug, identity) {
    make_sample("@x.x");
    std::string real_tree = "#(V)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, x_plus_y) {
    make_sample("+ x y");
    std::string real_tree = "@(@(*)(V))(V)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, plus_func) {
    make_sample("@x.@y.+ x y");
    std::string real_tree = "#(#(@(@(*)(V))(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, const_plus) {
    make_sample("+ 3 5");
    std::string real_tree = "@(@(*)(C))(C)";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, double_combinator) {
    make_sample("(@x.x x) @x.x x");
    std::string real_tree = "@(#(@(V)(V)))(#(@(V)(V)))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, double_combinator_if_no_parenthesis) {
    make_sample("@x.x x @x.x x");
    std::string real_tree = "#(@(@(V)(V))(#(@(V)(V))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}

TEST_F(ast_nondebug, y_combinator) {
    make_sample("@f.(@x.f(x x))(@x.f(x x))");
    std::string real_tree = "#(@(#(@(V)(@(V)(V))))(#(@(V)(@(V)(V)))))";
    size_t start = 0;
    ASSERT_EQ(yyparse(root), 0);
    Tree* real_root = new Tree(real_tree, start);
    real_root->check(real_root, root);
}