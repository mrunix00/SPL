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
    ASSERT_EQ((int64_t) vm.topStack(), 42);
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
    auto obj = (StringObject *) vm.topPointer();
    ASSERT_EQ(*obj, "Hello World");
}

TEST(VM, ListsDeclaration) {
    const char *input = "define x : int[] = [1, 2, 3, 4];"
                        "x;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    auto obj = (ArrayObject *) vm.topPointer();
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

TEST(VM, AppendToArray) {
    const char *input = "define x : int[] = [1, 2, 3, 4];"
                        "x += 5;"
                        "x;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    auto obj = (ArrayObject *) vm.topPointer();
    ASSERT_EQ(*obj, std::vector<uint64_t>({1, 2, 3, 4, 5}));
}

TEST(VM, VoidReturnType) {
    const char *input = "define x = 0;"
                        "define foo : function() -> void = { x = 42; };"
                        "foo();"
                        "x;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 42);
}

TEST(VM, TernaryExpressions) {
    const char *input = "define fib : function(n: int) -> int = { return n < 2 ? n : fib(n - 1) + fib(n - 2); };"
                        "fib(10);";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(vm.topStack(), 55);
}

TEST(VM, AddDecimalNumbers) {
    const char *input = "1.5 + 2.5;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 4);
}

TEST(VM, SubtractDecimalNumbers) {
    const char *input = "3.5 - 2.5;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 1);
}

TEST(VM, MultiplyDecimalNumbers) {
    const char *input = "3.5 * 2.5;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 8.75);
}

TEST(VM, DivideDecimalNumbers) {
    const char *input = "3.5 / 2.5;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 1.4);
}

TEST(VM, DeclareAFloatVariable) {
    const char *input = "define a : float = 42.42;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 42.42);
}

TEST(VM, FloatVariableAssignment) {
    const char *input = "define a : float = 42.42;"
                        "a = 43.43;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 43.43);
}

TEST(VM, FloatIncrementUnaryOperator) {
    const char *input = "define a : float = 42.42;"
                        "a++;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 43.42);
}

TEST(VM, FloatDecrementUnaryOperator) {
    const char *input = "define a : float = 42.42;"
                        "a--;"
                        "a;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 41.42);
}

TEST(VM, AddAFloatAndAnInteger) {
    const char *input = "42.42 + 42;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(std::bit_cast<double>(vm.topStack()), 84.42);
}
