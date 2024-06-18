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
    Define,
    If,
    Else,
    While,
    Return,
    U8,
    U16,
    U32,
    U64,
    I8,
    I16,
    I32,
    I64,
    F32,
    F64,
    Bool,
    True,
    False,
};

struct Token {
    TokenType type;
    std::string value;

    inline bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }
};
