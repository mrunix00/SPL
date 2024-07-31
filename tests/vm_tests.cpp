#include "ast.h"
#include <gtest/gtest.h>
#include <vector>

TEST(VM, SimpleAddition) {
    const char *input = "1 + 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 3);
}

TEST(VM, SimpleSubtraction) {
    const char *input = "3 - 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 1);
}

TEST(VM, SimpleMultiplication) {
    const char *input = "3 * 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 6);
}

TEST(VM, SimpleDivision) {
    const char *input = "6 / 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 3);
}

TEST(VM, SimpleModulus) {
    const char *input = "6 % 4;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 2);
}

TEST(VM, CompoundExpression) {
    const char *input = "1 + 2 * 3;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 7);
}

TEST(VM, SimpleVariableDeclaration) {
    const char *input = "define a : int = 42;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 42);
}

TEST(VM, VariablesShouldBeInitializedWithZero) {
    const char *input = "define a : int;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 0);
}

TEST(VM, TypeDeduction) {
    const char *input = "define a = 42;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 42);
}

TEST(VM, DeclareBooleans) {
    const char *input = "define a : bool = true;"
                        "define b : bool = false;"
                        "a == b;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 0);
}

TEST(VM, SimpleVariableAssignment) {
    const char *input = "define a : int = 42; a = 43; a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 43);
}

TEST(VM, RightIncrementUnaryOperator) {
    const char *input = "define a : int = 42;"
                        "a++;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 43);
}

TEST(VM, RightDecrementUnaryOperator) {
    const char *input = "define a : int = 42;"
                        "a--;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 41);
}

TEST(VM, LeftIncrementUnaryOperator) {
    const char *input = "define a : int = 42;"
                        "++a;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 43);
}

TEST(VM, IncrementAssign) {
    const char *input = "define a : int = 42;"
                        "a += 27;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 69);
}

TEST(VM, DecrementAssign) {
    const char *input = "define a : int = 69;"
                        "a -= 27;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 42);
}

TEST(VM, LeftDecrementUnaryOperator) {
    const char *input = "define a : int = 42;"
                        "--a;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 41);
}

TEST(VM, SimpleIfCondition) {
    const char *input = "define a : int = 69;"
                        "if 10 > 0 { a = 42; };"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 42);
}

TEST(VM, SimpleIfElseCondition) {
    const char *input = "define a : int = 69;"
                        "if 10 < 0 { a = 42; } else { a = 43; };"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 43);
}

TEST(VM, SimpleFunctionDeclaration) {
    const char *input = "define add : function(x: int, y: int) -> int = { return x + y; };"
                        "add(1, 2);";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 3);
}

TEST(VM, RecursiveFunction) {
    const char *input = "define fib : function(n: int) -> int = { if n < 2 { return n; } else { return fib(n - 1) + fib(n - 2); }; };"
                        "fib(10);";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 55);
}

TEST(VM, SimpleWhileLoop) {
    const char *input = "define a : int = 0;"
                        "while a < 10 { a = a + 1; };"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 10);
}

TEST(VM, SimpleForLoop) {
    const char *input = "define sum : int = 0;"
                        "for define i = 0; i < 10; i++ {"
                        "sum = sum + i;"
                        "};"
                        "sum;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 45);
}

TEST(VM, DeclareStrings) {
    const char *input = "define string : str = \"Hello World\";"
                        "string;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    auto obj = reinterpret_cast<StringObject *>(vm.topStack());
    ASSERT_EQ(*obj, "Hello World");
}

TEST(VM, ListsDeclaration) {
    const char *input = "define x : int[] = [1, 2, 3, 4];"
                        "x;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    auto obj = reinterpret_cast<ArrayObject *>(vm.topStack());
    ASSERT_EQ(*obj, std::vector<uint64_t>({1, 2, 3, 4}));
}

TEST(VM, ArrayAccess) {
    const char *input = "define x : int[] = [1, 2, 3, 4];"
                        "x[2];";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 3);
}

TEST(VM, AppendToArray){
    const char *input = "define x : int[] = [1, 2, 3, 4];"
                        "x += 5;"
                        "x;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    auto obj = reinterpret_cast<ArrayObject *>(vm.topStack());
    ASSERT_EQ(*obj, std::vector<uint64_t>({1, 2, 3, 4, 5}));
}
