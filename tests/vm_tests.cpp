#include "ast.h"
#include <gtest/gtest.h>

TEST(VM, SimpleAddition) {
    const char *input = "1 + 2;";
    VM vm;
    auto program = compile(input);
    vm.run(program);
    ASSERT_EQ(*static_cast<int32_t *>(vm.popStack(sizeof(int32_t))), 3);
}