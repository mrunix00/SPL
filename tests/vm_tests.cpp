#include "ast.h"
#include <gtest/gtest.h>

TEST(VM, SimpleAddition) {
    const char *input = "1 + 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 3);
}

TEST(VM, SimpleSubtraction) {
    const char *input = "3 - 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 1);
}

TEST(VM, SimpleMultiplication) {
    const char *input = "3 * 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 6);
}

TEST(VM, SimpleDivision) {
    const char *input = "6 / 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 3);
}

TEST(VM, SimpleModulus) {
    const char *input = "6 % 4;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 2);
}

TEST(VM, CompoundExpression) {
    const char *input = "1 + 2 * 3;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 7);
}

TEST(VM, SimpleVariableDeclaration) {
    const char *input = "define a : i32 = 42;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.getGlobal(0)), 42);
}

TEST(VM, SimpleVariableAssignment) {
    const char *input = "define a : i32 = 42; a = 43;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.getGlobal(0)), 43);
}

TEST(VM, SimpleIfCondition) {
    const char *input = "define a : i32 = 69;"
                        "if 10 > 0 { a = 42; };";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.getGlobal(0)), 42);
}

TEST(VM, SimpleIfElseCondition) {
    const char *input = "define a : i32 = 69;"
                        "if 10 < 0 { a = 42; } else { a = 43; };";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.getGlobal(0)), 43);
}

TEST(VM, SimpleFunctionDeclaration) {
    const char *input = "define add : function(x: i32, y: i32) -> i32 = { return x + y; };"
                        "add(1, 2);";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 3);
}

TEST(VM, RecursiveFunction) {
    const char *input = "define fib : function(n: i32) -> i32 = { if n < 2 { return n; } else { return fib(n - 1) + fib(n - 2); }; };"
                        "fib(10);";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.topStack(sizeof(int32_t))), 55);
}
