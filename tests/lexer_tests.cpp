#include "lexer.h"
#include "token.h"
#include <gtest/gtest.h>

static inline std::vector<Token> lex(const char *input) {
    std::vector<Token> tokens;
    auto buffer = yy_scan_string(input);
    while (true) {
        auto token = yylex();
        tokens.push_back({
                static_cast<TokenType>(token),
                yytext,
        });
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
    std::vector<Token> expected = {
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Number) {
    auto actual = lex("42");
    std::vector<Token> expected = {
            {TokenType::Number, "42"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Identifier) {
    auto actual = lex("foo");
    std::vector<Token> expected = {
            {TokenType::Identifier, "foo"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, String) {
    auto actual = lex("\"foo\"");
    std::vector<Token> expected = {
            {TokenType::String, "\"foo\""},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, ArithmeticOperators) {
    auto actual = lex("+-*/%=");
    std::vector<Token> expected = {
            {TokenType::Plus, "+"},
            {TokenType::Minus, "-"},
            {TokenType::Multiply, "*"},
            {TokenType::Divide, "/"},
            {TokenType::Modulo, "%"},
            {TokenType::Assign, "="},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, BooleanOperators) {
    auto actual = lex("== != < <= > >= && || !");
    std::vector<Token> expected = {
            {TokenType::Equal, "=="},
            {TokenType::NotEqual, "!="},
            {TokenType::Less, "<"},
            {TokenType::LessEqual, "<="},
            {TokenType::Greater, ">"},
            {TokenType::GreaterEqual, ">="},
            {TokenType::And, "&&"},
            {TokenType::Or, "||"},
            {TokenType::Not, "!"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Newline) {
    auto actual = lex("1\n2");
    std::vector<Token> expected = {
            {TokenType::Number, "1"},
            {TokenType::Number, "2"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Whitespace) {
    auto actual = lex("1 2");
    std::vector<Token> expected = {
            {TokenType::Number, "1"},
            {TokenType::Number, "2"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, Comments) {
    auto actual = lex("1 // foo\n2");
    std::vector<Token> expected = {
            {TokenType::Number, "1"},
            {TokenType::Number, "2"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}

TEST(Lexer, MultiLineComments) {
    auto actual = lex("1 /* foo\nbar */ 2");
    std::vector<Token> expected = {
            {TokenType::Number, "1"},
            {TokenType::Number, "2"},
            {TokenType::EndOfFile},
    };
    TEST_LEX(actual, expected)
}
