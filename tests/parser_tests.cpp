#include <gtest/gtest.h>
#include <vector>

#include "ast.h"
#include "parser.h"

static void deleteAST(std::vector<AbstractSyntaxTree *> &ast) {
    for (auto &node : ast)
        delete node;
}

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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ParseStrings) {
    const char *input = "x = \"foo\";";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new BinaryExpression(
                    new Node({Identifier, "x"}),
                    new Node({String, "foo"}),
                    {Assign, "="}),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    ASSERT_EQ(*expectedResult[0], *actualResult[0]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, Declaration) {
    const char *input = "define a : uint;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new Node({UInt, "uint"}),
                    Node({Identifier, "a"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, DeclarationWithInitialization) {
    const char *input = "define a : uint = 42;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new Node({UInt, "uint"}),
                    Node({Identifier, "a"}),
                    new Node({Number, "42"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, FunctionDeclaration) {
    const char *input = "define max : function(x: int, y: int) -> int;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new FunctionDeclaration(
                            new Node({Int, "int"}),
                            {new Declaration(
                                     new Node({Int, "int"}),
                                     Node({Identifier, "x"})),
                             new Declaration(
                                     new Node({Int, "int"}),
                                     Node({Identifier, "y"}))}),
                    Node({Identifier, "max"})),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, FunctionDeclarationWithBody) {
    const char *input = "define max : function(x: int, y: int) -> int = {\n"
                        "\treturn x + y;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new FunctionDeclaration(
                            new Node({Int, "int"}),
                            {
                                    new Declaration(
                                            new Node({Int, "int"}),
                                            Node({Identifier, "x"})),
                                    new Declaration(
                                            new Node({Int, "int"}),
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, FunctionDeclarationWithBodyWithMultipleExpressions) {
    const char *input = "define max : function(x: int, y: int) -> int = {\n"
                        "\tdefine result : int = x + y;\n"
                        "\treturn result;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new Declaration(
                    new FunctionDeclaration(
                            new Node({Int, "int"}),
                            {
                                    new Declaration(
                                            new Node({Int, "int"}),
                                            Node({Identifier, "x"})),
                                    new Declaration(
                                            new Node({Int, "int"}),
                                            Node({Identifier, "y"})),
                            }),
                    Node({Identifier, "max"}),
                    new ScopedBody({
                            new Declaration(
                                    new Node({Int, "int"}),
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, FunctionDeclarationWithAutoTypeDeduction) {
    const char *input = "define max = (function (x: int, y: int) -> int) {\n"
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
                                    new Node({Int, "int"}),
                                    {
                                            new Declaration(
                                                    new Node({Int, "int"}),
                                                    Node({Identifier, "x"})),
                                            new Declaration(
                                                    new Node({Int, "int"}),
                                                    Node({Identifier, "y"})),
                                    }))),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ForLoop) {
    const char *input = "for i = 0; i < 10; i++ {\n"
                        "\treturn i;\n"
                        "};";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new ForLoop(
                    new BinaryExpression(
                            new Node({Identifier, "i"}),
                            new Node({Number, "0"}),
                            {Assign, "="}),
                    new BinaryExpression(
                            new Node({Identifier, "i"}),
                            new Node({Number, "10"}),
                            {Less, "<"}),
                    new UnaryExpression(
                            new Node({Identifier, "i"}),
                            {Increment, "++"},
                            UnaryExpression::Side::RIGHT),
                    new ScopedBody({
                            new ReturnStatement(new Node({Identifier, "i"})),
                    })),
    };
    auto actualResult = parse(input);

    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
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
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, List) {
    const char *input = "[1, 2, 3, 4];";
    auto expectedResult = std::vector<AbstractSyntaxTree *>{
            new List({
                    new Node({Number, "1"}),
                    new Node({Number, "2"}),
                    new Node({Number, "3"}),
                    new Node({Number, "4"}),
            }),
    };
    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ArrayDeclaration) {
    const char *input = "define x : int[] = [1, 2, 3];";
    auto expectedResult = std::vector<AbstractSyntaxTree *>({new Declaration(
            new ArrayType(new Node({Int, "int"})),
            Node({Identifier, "x"}),
            new List({
                    new Node({Number, "1"}),
                    new Node({Number, "2"}),
                    new Node({Number, "3"}),
            }))});

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ArrayAccess) {
    const char *input = "x[0];";
    auto expectedResult = std::vector<AbstractSyntaxTree *>({
            new ArrayAccess(
                    Node({Identifier, "x"}),
                    new Node({Number, "0"})),
    });

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, NestedScopes) {
    const char *input =
            "define fun : function(x : int) -> int = {"
            "   define i = 0;"
            "   define n = 0;"
            "   while n < 128 {"
            "       i += n;"
            "       n++;"
            "   };"
            "   return i;"
            "};";

    auto expectedResult = std::vector<AbstractSyntaxTree *>({
            new Declaration(
                    new FunctionDeclaration(
                            new Node({Int, "int"}),
                            {
                                    new Declaration(
                                            new Node({Int, "int"}),
                                            Node({Identifier, "x"})),
                            }),
                    Node({Identifier, "fun"}),
                    new ScopedBody({
                            new Declaration(
                                    Node({Identifier, "i"}),
                                    new Node({Number, "0"})),
                            new Declaration(
                                    Node({Identifier, "n"}),
                                    new Node({Number, "0"})),
                            new WhileStatement(
                                    new BinaryExpression(
                                            new Node({Identifier, "n"}),
                                            new Node({Number, "128"}),
                                            {Less, "<"}),
                                    new ScopedBody({
                                            new BinaryExpression(
                                                    new Node({Identifier, "i"}),
                                                    new Node({Identifier, "n"}),
                                                    {IncrementAssign, "+="}),
                                            new UnaryExpression(
                                                    new Node({Identifier, "n"}),
                                                    {Increment, "++"},
                                                    UnaryExpression::Side::RIGHT),
                                    })),
                            new ReturnStatement(new Node({Identifier, "i"})),
                    })),
    });

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ImportStatement) {
    const char *input = "import \"foo\";";
    auto expectedResult = std::vector<AbstractSyntaxTree *>({
            new ImportStatement("foo"),
    });

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, ExportStatement) {
    const char *input = "export define fun : function() -> void = { 1+1; };";
    auto expectedResult = std::vector<AbstractSyntaxTree *>({
            new ExportStatement(
                    new Declaration(
                            new FunctionDeclaration(new Node({Void, "void"}), {}),
                            Node({Identifier, "fun"}),
                            new ScopedBody({
                                    new BinaryExpression(
                                            new Node({Number, "1"}),
                                            new Node({Number, "1"}),
                                            {Plus, "+"}),
                            }))),
    });

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}

TEST(ParserTests, TurnaryExpressions) {
    const char *input = "x == 0 ? 1 : 0;";
    auto expectedResult = std::vector<AbstractSyntaxTree *>({
            new TernaryExpression(
                    new BinaryExpression(
                            new Node({Identifier, "x"}),
                            new Node({Number, "0"}),
                            {Equal, "=="}),
                    new Node({Number, "1"}),
                    new Node({Number, "0"})
            ),
    });

    auto actualResult = parse(input);
    ASSERT_EQ(expectedResult.size(), actualResult.size());
    for (int i = 0; i < expectedResult.size(); i++)
        ASSERT_EQ(*expectedResult[i], *actualResult[i]);
    deleteAST(actualResult);
    deleteAST(expectedResult);
}
