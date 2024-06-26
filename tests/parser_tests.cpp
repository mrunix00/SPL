#include <gtest/gtest.h>

#include "ast.h"
#include "parser.h"

TEST(ParserTests, SimpleBinaryExpression) {
    const char *input = "1 + 2;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new Node({Number, "1"}),
                    new Node({Number, "2"}),
                    {Plus, "+"}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, CompoundBinaryExpression) {
    const char *input = "1 + 2 * 3;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new Node({Number, "1"}),
                    new BinaryExpression(
                            new Node({Number, "2"}),
                            new Node({Number, "3"}),
                            {Multiply, "*"}),
                    {Plus, "+"}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, MultipleExpressions) {
    const char *input = "1 + 2; 3 + 4; 5 + 6;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new Node({Number, "1"}),
                    new Node({Number, "2"}),
                    {Plus, "+"}),
            new BinaryExpression(
                    new Node({Number, "3"}),
                    new Node({Number, "4"}),
                    {Plus, "+"}),
            new BinaryExpression(
                    new Node({Number, "5"}),
                    new Node({Number, "6"}),
                    {Plus, "+"}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, OperationsPriority) {
    const char *input = "1 * 2 + 3;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new BinaryExpression(
                            new Node({Number, "1"}),
                            new Node({Number, "2"}),
                            {Multiply, "*"}),
                    new Node({Number, "3"}),
                    {Plus, "+"}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, Declaration) {
    const char *input = "define a : u32;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new Node({U32, "u32"}),
                    Node({Identifier, "a"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, DeclarationWithInitialization) {
    const char *input = "define a : u32 = 42;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new Node({U32, "u32"}),
                    Node({Identifier, "a"}),
                    new Node({Number, "42"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, DeclarationWithAutoTypeDeduction) {
    const char *input = "define a = 42;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    Node({Identifier, "a"}),
                    new Node({Number, "42"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, VariableAssignment) {
    const char *input = "a = 42;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new Node({Identifier, "a"}),
                    new Node({Number, "42"}),
                    {Assign, "="}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionDeclaration) {
    const char *input = "define max : function(x: i32, y: i32) -> i32;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new FunctionDeclaration(
                            new Node({I32, "i32"}),
                            {new Declaration(
                                     new Node({I32, "i32"}),
                                     Node({Identifier, "x"})),
                             new Declaration(
                                     new Node({I32, "i32"}),
                                     Node({Identifier, "y"}))}),
                    Node({Identifier, "max"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionDeclarationWithBody) {
    const char *input = "define max : function(x: i32, y: i32) -> i32 = {\n"
                        "\treturn x + y;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
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
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionDeclarationWithBodyWithMultipleExpressions) {
    const char *input = "define max : function(x: i32, y: i32) -> i32 = {\n"
                        "\tdefine result : i32 = x + y;\n"
                        "\treturn result;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
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
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionDeclarationWithAutoTypeDeduction) {
    const char *input = "define max = (function (x: i32, y: i32) -> i32) {\n"
                        "\treturn x + y;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
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
                                    }))),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionCall) {
    const char *input = "max();";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new FunctionCall(
                    Node({Identifier, "max"}),
                    {}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionCallWithArgument) {
    const char *input = "max(x);";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new FunctionCall(
                    Node({Identifier, "max"}),
                    {
                            new Node({Identifier, "x"}),
                    }),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, FunctionCallWithMultipleArguments) {
    const char *input = "max(1, 2);";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new FunctionCall(
                    Node({Identifier, "max"}),
                    {
                            new Node({Number, "1"}),
                            new Node({Number, "2"}),
                    }),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, IfStatement) {
    const char *input = "if x {\n"
                        "\treturn x;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new IfStatement(
                    new Node({Identifier, "x"}),
                    new ScopedBody({
                            new ReturnStatement(new Node({Identifier, "x"})),
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, IfElseStatement) {
    const char *input = "if x {\n"
                        "\treturn x;\n"
                        "} else {\n"
                        "\treturn y;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new IfStatement(
                    new Node({Identifier, "x"}),
                    new ScopedBody({
                            new ReturnStatement(new Node({Identifier, "x"})),
                    }),
                    new ScopedBody({
                            new ReturnStatement(new Node({Identifier, "y"})),
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, WhileStatement) {
    const char *input = "while x {\n"
                        "\treturn x;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new WhileStatement(
                    new Node({Identifier, "x"}),
                    new ScopedBody({
                            new ReturnStatement(new Node({Identifier, "x"})),
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}

TEST(ParserTests, UnaryExpression) {
    const char *input = "x++;"
                        "x--;"
                        "++x;"
                        "--x;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new UnaryExpression(
                    new Node({Identifier, "x"}),
                    {Increment, "++"},
                    UnaryExpression::Side::RIGHT),
            new UnaryExpression(
                    new Node({Identifier, "x"}),
                    {Decrement, "--"},
                    UnaryExpression::Side::RIGHT),
            new UnaryExpression(
                    new Node({Identifier, "x"}),
                    {Increment, "++"},
                    UnaryExpression::Side::LEFT),
            new UnaryExpression(
                    new Node({Identifier, "x"}),
                    {Decrement, "--"},
                    UnaryExpression::Side::LEFT),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
}
