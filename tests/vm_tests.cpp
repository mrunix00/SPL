#include "ast.h"
#include <gtest/gtest.h>

TEST(VM, SimpleAddition) {
    const char *input = "1 + 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 3);
}

TEST(VM, SimpleSubtraction) {
    const char *input = "3 - 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 1);
}

TEST(VM, SimpleMultiplication) {
    const char *input = "3 * 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 6);
}

TEST(VM, SimpleDivision) {
    const char *input = "6 / 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 3);
}

TEST(VM, SimpleModulus) {
    const char *input = "6 % 4;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 2);
}

TEST(VM, CompoundExpression) {
    const char *input = "1 + 2 * 3;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 7);
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
