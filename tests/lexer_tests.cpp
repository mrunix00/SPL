#include "lexer.h"
#include "token.h"
#include <gtest/gtest.h>

static inline std::vector<Token> lex(const char *input) {
    std::vector<Token> tokens;
    auto buffer = yy_scan_string(input);
    while (true) {
        auto token = yylex();
        tokens.push_back({token, yytext});
        if (token == 0)
            break;
    }
    yy_delete_buffer(buffer);
    return tokens;
}

#define TEST_LEX(actual, expected)                     \
    ASSERT_EQ(actual.size(), expected.size());         \
    for (size_t i = 0; i < actual.size(); i++) {       \
        ASSERT_EQ(actual[i].type, expected[i].type);   \
        ASSERT_EQ(actual[i].value, expected[i].value); \
    }

TEST(Lexer, EmptyString) {
    auto actual = lex("");
    std::vector<Token> expected = {{YYEOF}};
    TEST_LEX(actual, expected)
}

TEST(Lexer, Number) {
    auto actual = lex("42");
    std::vector<Token> expected = {
            {Number, "42"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Identifier) {
    auto actual = lex("foo");
    std::vector<Token> expected = {
            {Identifier, "foo"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, String) {
    auto actual = lex("\"foo\"");
    std::vector<Token> expected = {
            {String, "\"foo\""},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, ArithmeticOperators) {
    auto actual = lex("+-*/%=++--+=-=");
    std::vector<Token> expected = {
            {Plus, "+"},
            {Minus, "-"},
            {Multiply, "*"},
            {Divide, "/"},
            {Modulo, "%"},
            {Assign, "="},
            {Increment, "++"},
            {Decrement, "--"},
            {IncrementAssign, "+="},
            {DecrementAssign, "-="},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, BooleanOperators) {
    auto actual = lex("== != < <= > >= && || !");
    std::vector<Token> expected = {
            {Equal, "=="},
            {NotEqual, "!="},
            {Less, "<"},
            {LessEqual, "<="},
            {Greater, ">"},
            {GreaterEqual, ">="},
            {And, "&&"},
            {Or, "||"},
            {Not, "!"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Symbols) {
    auto actual = lex(":;(){}[]->");
    std::vector<Token> expected = {
            {Colon, ":"},
            {Semicolon, ";"},
            {LParen, "("},
            {RParen, ")"},
            {LBrace, "{"},
            {RBrace, "}"},
            {LBracket, "["},
            {RBracket, "]"},
            {Arrow, "->"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Keywords) {
    auto actual = lex(
            "define if else while for return "
            "u8 u16 u32 u64 i8 i16 i32 i64 "
            "f32 f64 bool true false");

    std::vector<Token> expected = {
            {Define, "define"},
            {If, "if"},
            {Else, "else"},
            {While, "while"},
            {For, "for"},
            {Return, "return"},
            {U8, "u8"},
            {U16, "u16"},
            {U32, "u32"},
            {U64, "u64"},
            {I8, "i8"},
            {I16, "i16"},
            {I32, "i32"},
            {I64, "i64"},
            {F32, "f32"},
            {F64, "f64"},
            {Bool, "bool"},
            {True, "true"},
            {False, "false"},
            {YYEOF},
    };

    TEST_LEX(actual, expected)
}

TEST(Lexer, Newline) {
    auto actual = lex("1\n2");
    std::vector<Token> expected = {
            {Number, "1"},
            {Number, "2"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Whitespace) {
    auto actual = lex("1 2");
    std::vector<Token> expected = {
            {Number, "1"},
            {Number, "2"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Comments) {
    auto actual = lex("1 // foo\n2");
    std::vector<Token> expected = {
            {Number, "1"},
            {Number, "2"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, MultiLineComments) {
    auto actual = lex("1 /* foo\nbar */ 2");
    std::vector<Token> expected = {
            {Number, "1"},
            {Number, "2"},
            {YYEOF},
    };
    TEST_LEX(actual, expected)
}
