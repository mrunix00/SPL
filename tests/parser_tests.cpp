#include <gtest/gtest.h>

#include "ast.h"
#include "parser.h"

TEST(ParserTests, SimpleBinaryExpression) {
    const char *input = "1 + 2;";
    auto expectedResult = new BinaryExpression(
            new Node({Number, "1"}),
            new Node({Number, "2"}),
            {Plus, "+"});
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, CompoundBinaryExpression) {
    const char *input = "1 + 2 * 3;";
    auto expectedResult = new BinaryExpression(
            new Node({Number, "1"}),
            new BinaryExpression(
                    new Node({Number, "2"}),
                    new Node({Number, "3"}),
                    {Multiply, "*"}),
            {Plus, "+"});
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, OperationsPriority) {
    const char *input = "1 * 2 + 3;";
    auto expectedResult = new BinaryExpression(
            new BinaryExpression(
                    new Node({Number, "1"}),
                    new Node({Number, "2"}),
                    {Multiply, "*"}),
            new Node({Number, "3"}),
            {Plus, "+"});
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, Declaration) {
    const char *input = "define a : u32;";
    auto expectedResult = new Declaration(
            new Node({U32, "u32"}),
            Node({Identifier, "a"}));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, DeclarationWithInitialization) {
    const char *input = "define a : u32 = 42;";
    auto expectedResult = new Declaration(
            new Node({U32, "u32"}),
            Node({Identifier, "a"}),
            new Node({Number, "42"}));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, DeclarationWithAutoTypeDeduction) {
    const char *input = "define a = 42;";
    auto expectedResult = new Declaration(
            Node({Identifier, "a"}),
            new Node({Number, "42"}));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, FunctionDeclaration) {
    const char *input = "define max : function(x: i32, y: i32) -> i32;";
    auto expectedResult = new Declaration(
            new FunctionDeclaration(
                    new Node({I32, "i32"}),
                    {new Declaration(
                             new Node({I32, "i32"}),
                             Node({Identifier, "x"})),
                     new Declaration(
                             new Node({I32, "i32"}),
                             Node({Identifier, "y"}))}),
            Node({Identifier, "max"}));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, FunctionDeclarationWithBody) {
    const char *input = "define max : function(x: i32, y: i32) -> i32 = {\n"
                        "\treturn x + y\n"
                        "};";
    auto expectedResult = new Declaration(
            new FunctionDeclaration(
                    new Node({I32, "i32"}),
                    {
                            new Declaration(
                                    new Node({I32, "i32"}),
                                    Node({Identifier, "x"})),
                            new Declaration(
                                    new Node({I32, "i32"}),
                                    Node({Identifier, "y"})),
                    }),
            Node({Identifier, "max"}),
            new ScopedBody({
                    new ReturnStatement(new BinaryExpression(
                            new Node({Identifier, "x"}),
                            new Node({Identifier, "y"}),
                            {Plus, "+"})),
            }));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, FunctionDeclarationWithBodyWithMultipleExpressions) {
    const char *input = "define max : function(x: i32, y: i32) -> i32 = {\n"
                        "\tdefine result : i32 = x + y;\n"
                        "\treturn result\n"
                        "};";
    auto expectedResult = new Declaration(
            new FunctionDeclaration(
                    new Node({I32, "i32"}),
                    {
                            new Declaration(
                                    new Node({I32, "i32"}),
                                    Node({Identifier, "x"})),
                            new Declaration(
                                    new Node({I32, "i32"}),
                                    Node({Identifier, "y"})),
                    }),
            Node({Identifier, "max"}),
            new ScopedBody({
                    new Declaration(
                            new Node({I32, "i32"}),
                            Node({Identifier, "result"}),
                            new BinaryExpression(
                                    new Node({Identifier, "x"}),
                                    new Node({Identifier, "y"}),
                                    {Plus, "+"})),
                    new ReturnStatement(new Node({Identifier, "result"})),
            }));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}

TEST(ParserTests, FunctionDeclarationWithAutoTypeDeduction) {
    const char *input = "define max = (function (x: i32, y: i32) -> i32) {\n"
                        "\treturn x + y\n"
                        "};";
    auto expectedResult = new Declaration(
            Node({Identifier, "max"}),
            new TypeCast(
                    new ScopedBody({
                            new ReturnStatement(new BinaryExpression(
                                    new Node({Identifier, "x"}),
                                    new Node({Identifier, "y"}),
                                    {Plus, "+"})),
                    }),
                    new FunctionDeclaration(
                            new Node({I32, "i32"}),
                            {
                                    new Declaration(
                                            new Node({I32, "i32"}),
                                            Node({Identifier, "x"})),
                                    new Declaration(
                                            new Node({I32, "i32"}),
                                            Node({Identifier, "y"})),
                            })));
    auto actualResult = parse(input);

    ASSERT_EQ(*expectedResult, *actualResult);
    delete expectedResult;
    delete actualResult;
}
