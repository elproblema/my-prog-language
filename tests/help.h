#pragma once

#include <gtest/gtest.h>

#include <cassert>
#include <map>
#include <functional>

#include <parser.hpp>
#include <scanner.hpp>
#include <ast/ast.h>
#include <ast/SC.h>

// ###################### HELPING STRUCTURES #########################

struct Tree;
extern int yy_flex_debug;

extern std::map<char, std::function<void(std::shared_ptr<Tree>, std::shared_ptr<const Node>)>> char_map_func; //NOLINT

struct Base {
    using Type = Node;
    virtual void check(const Node&) = 0;

    const Type* skip_ind_nodes(const Type& obj) {
        const auto* ptr = &obj;
        while (auto ptr_ = dynamic_cast<const LambdaNode*>(ptr)) {
            if (!ptr_->ind_tag) {
                break;
            }
            ptr = ptr_->body.get();
        }
        return ptr;
    }

    virtual ~Base() {}
};

// ####################### METAPROGRAMMING ##########################

template<typename T>
struct member_pointer_class;

template<typename Class, typename Value>
struct member_pointer_class<Value Class::*>
{
    typedef Class type;
};

template<typename T>
struct member_pointer_value;

template<typename Class, typename Value>
struct member_pointer_value<Value Class::*>
{
    typedef Value type;
};

template<typename T>
concept LambdaLike = requires {
    &T::operator();
};

template<typename T>
struct function_traits {};

template<LambdaLike T>
struct function_traits<T> : function_traits<decltype(&T::operator())> {
};

template<class R, class... Args>
struct function_traits<R(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template<class R, class... Args>
struct function_traits<R (*)(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template<class R, class... Args>
struct function_traits<std::function<R(Args...)>> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args...)> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template<class T, class R, class... Args>
struct function_traits<R (T::*)(Args...) const> {
    using result_type = R;
    using argument_types = std::tuple<Args...>;
};

template<typename T>
concept Callable = requires {
    typename function_traits<T>::result_type;
};

template<typename T>
concept MemberPointer = std::is_member_pointer_v<T>;

template<typename T, typename U>
concept Derived = std::is_base_of_v<U, T>;


template<typename ...Args>
struct Wrap;

template <
MemberPointer PtrToMemberType, 
typename Value,
typename ...Tail>
struct Wrap<PtrToMemberType, Value, Tail...> : public Base {
    using class_t = member_pointer_class<PtrToMemberType>::type;

    Value value;
    PtrToMemberType ptr;
    Wrap<Tail...> other;

    Wrap(PtrToMemberType ptr, Value x, Tail... args): ptr(ptr), value(x), other(args...) {}

    void check(const Type& obj__) {
        const Type* obj_ = skip_ind_nodes(obj__);
        ASSERT_NO_THROW(dynamic_cast<const class_t&>(*obj_));
        const class_t& obj = dynamic_cast<const class_t&>(*obj_);
        if constexpr (std::is_member_function_pointer<PtrToMemberType>::value) {
            ASSERT_EQ(value, (obj.*ptr)());
        } else {
            ASSERT_EQ(value, obj.*ptr);
        }
        other.check(obj);
    }

};

template<Derived<Node> Class>
struct Wrap<Class> : public Base {
    Wrap() {}

    void check(const Type& obj__) {
        const Type* obj_ = skip_ind_nodes(obj__);
        ASSERT_NO_THROW(dynamic_cast<const Class&>(*obj_));
    }
};

template<Callable Func, typename ...Tail>
struct Wrap<Func, Tail...> : public Base {
    using class_t = typename std::tuple_element<0, typename function_traits<Func>::argument_types>::type;

    Func f;
    Wrap<Tail...> other;

    Wrap(Func f, Tail... args): f(f), other(args...) {}

    void check(const Type& obj__) {
        const Type* obj_ = skip_ind_nodes(obj__);
        ASSERT_NO_THROW(dynamic_cast<const class_t&>(*obj_));
        const class_t& obj = dynamic_cast<const class_t&>(*obj_);
        f(obj);
        other.check(obj);
    }
};

template<>
struct Wrap<> : public Base {
    using V = bool;
    Wrap() {}

    void check(const Type& obj_) {}
};


Wrap() -> Wrap<>;

template<
MemberPointer T, 
typename U, 
typename ...TT>
Wrap(T ptr, U x, TT... args) -> Wrap<T, U, TT...>;

template<Callable Func,  
typename ...Tail>
Wrap(Func f, Tail... args) -> Wrap<Func, Tail...>;


// ####################### TREE #############################

struct Tree {
    std::function<void(std::shared_ptr<Tree>, std::shared_ptr<const Node>)> check;
    std::vector<std::shared_ptr<Tree>> children;
    Base* template_;
    bool tag = false;

    size_t parse_index(const std::string& format, size_t& start) {
        size_t n = format.size();
        int res = -1;
        while ('0' <= format[start] && format[start] <= '9' && start < n) {
            res = std::max(0, res);
            res *= 10;
            res += format[start] - '0';
            ++start;
        }
        return res;
    }

    Tree(const std::string& format, size_t& start, const std::vector<Base*>& types) {
        size_t n = format.size();
        assert(n > start);
        char c = format[start];
        assert(c == '@' || c == 'V' || c == '*' || c == 'C' || c == '#' || c == 'S');

        check = char_map_func[c];

        ++start;
        int ind = parse_index(format, start);

        if (ind == -1) template_ = new Wrap(), tag = true;
        else template_ = types[ind];

        while (format[start] == '(' && start < n) {
            ++start;
            children.push_back(std::make_shared<Tree>(format, start, types));
        }
        if (format[start] == ')') { 
            ++start;
            return;
        }
    }

    ~Tree() {
        if (tag) delete template_;
    }
};

void make_sample(const char* s);

void set_map();

void unset_map();