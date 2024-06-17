#pragma once

#include <string>

enum TokenType {
    EndOfFile,
    Number,
    Identifier,
    String,
    Plus,
    Minus,
    Multiply,
    Divide,
    Modulo,
    Assign,
    Equal,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    And,
    Or,
    Not,
};

struct Token {
    TokenType type;
    std::string value;
};
