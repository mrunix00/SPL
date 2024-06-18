#include <gtest/gtest.h>

#include "ast.h"

TEST(ParserTests, SimpleBinaryExpression) {
    const char *input = "1 + 2";
    auto expectedResult = new BinaryExpression(
            new Node({TokenType::Number, "1"}),
            new Node({TokenType::Number, "2"}),
            {TokenType::Plus, "+"});
    delete expectedResult;
}

TEST(ParserTests, CompoundBinaryExpression) {
    const char *input = "1 + 2 * 3";
    auto expectedResult = new BinaryExpression(
            new Node({TokenType::Number, "1"}),
            new BinaryExpression(
                    new Node({TokenType::Number, "2"}),
                    new Node({TokenType::Number, "3"}),
                    {TokenType::Multiply, "*"}),
            {TokenType::Plus, "+"});
    delete expectedResult;
}

TEST(ParserTests, Declaration) {
    const char *input = "define a : u32";
    auto expectedResult = new Declaration(
            new Node({TokenType::Define, "u32"}),
            Node({TokenType::Identifier, "a"}));
    delete expectedResult;
}

TEST(ParserTests, DeclarationWithInitialization) {
    const char *input = "define a : u32 = 42";
    auto expectedResult = new Declaration(
            new Node({TokenType::Define, "u32"}),
            Node({TokenType::Identifier, "a"}),
            new Node({TokenType::Number, "42"}));
    delete expectedResult;
}

TEST(ParserTests, FunctionDeclaration) {
    const char *input = "define max : function(x: i32, y: i32) -> i32";
    auto expectedResult = new Declaration(
            new FunctionDeclaration(
                    new Node({TokenType::Define, "i32"}),
                    {new Declaration(
                             new Node({TokenType::Define, "i32"}),
                             Node({TokenType::Identifier, "x"})),
                     new Declaration(
                             new Node({TokenType::Define, "i32"}),
                             Node({TokenType::Identifier, "y"}))}),
            Node({TokenType::Identifier, "max"}));
    delete expectedResult;
}

TEST(ParserTests, FunctionDeclarationWithBody) {
    const char *input = "define max : function(x: i32, y: i32) -> i32 = {\n"
                        "\treturn x + y\n"
                        "}";
    auto expectedResult = new Declaration(
            new FunctionDeclaration(
                    new Node({TokenType::Define, "i32"}),
                    {
                            new Declaration(
                                    new Node({TokenType::Define, "i32"}),
                                    Node({TokenType::Identifier, "x"})),
                            new Declaration(
                                    new Node({TokenType::Define, "i32"}),
                                    Node({TokenType::Identifier, "y"})),
                    }),
            Node({TokenType::Identifier, "max"}),
            new ScopedBody({
                    new ReturnStatement(new BinaryExpression(
                            new Node({TokenType::Identifier, "x"}),
                            new Node({TokenType::Identifier, "y"}),
                            {TokenType::Plus, "+"})),
            }));
    delete expectedResult;
}
