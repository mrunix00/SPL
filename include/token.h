#pragma once

#include <string>
#include "parser.h"

struct Token {
    int type;
    std::string value;

    inline bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }
};
