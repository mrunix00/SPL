#pragma once

#include "token.h"
#include <vector>
#include <optional>

struct AbstractSyntaxTree {
    enum class Type {
        Invalid,
        Node,
        BinaryExpression,
        Declaration,
        ScopedBody,
        FunctionDeclaration,
        ReturnStatement
    } nodeType{Type::Invalid};
    virtual ~AbstractSyntaxTree() = default;
    virtual bool operator==(const AbstractSyntaxTree &other) const = 0;
};

struct Node final : public AbstractSyntaxTree {
    Token token;
    explicit Node(Token token);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct BinaryExpression final : public AbstractSyntaxTree {
    AbstractSyntaxTree *left{};
    AbstractSyntaxTree *right{};
    Token op;
    BinaryExpression(AbstractSyntaxTree *left, AbstractSyntaxTree *right, Token op);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct Declaration final : public AbstractSyntaxTree {
    AbstractSyntaxTree *type;
    Node identifier;
    std::optional<AbstractSyntaxTree *> value;
    Declaration(AbstractSyntaxTree *type, Node identifier, AbstractSyntaxTree *value);
    Declaration(AbstractSyntaxTree *type, Node identifier);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct ScopedBody : public AbstractSyntaxTree {
    std::vector<AbstractSyntaxTree *> body;
    explicit ScopedBody(const std::vector<AbstractSyntaxTree *> &body);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct FunctionDeclaration : public AbstractSyntaxTree {
    AbstractSyntaxTree *returnType;
    std::vector<Declaration *> arguments;
    FunctionDeclaration(AbstractSyntaxTree *returnType, const std::vector<Declaration *> &arguments);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct ReturnStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *expression;
    explicit ReturnStatement(AbstractSyntaxTree *expression);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

AbstractSyntaxTree* parse(const char *input);
