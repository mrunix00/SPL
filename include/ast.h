#pragma once

#include "token.h"
#include "vm.h"
#include <optional>
#include <stdexcept>
#include <vector>

struct AbstractSyntaxTree {
    std::string typeStr = "AbstractSyntaxTree";
    enum class Type {
        Invalid,
        Node,
        UnaryExpression,
        BinaryExpression,
        Declaration,
        ScopedBody,
        FunctionDeclaration,
        ReturnStatement,
        TypeCast,
        FunctionCall,
        IfStatement,
        WhileStatement,
        ForLoop,
    } nodeType{Type::Invalid};
    virtual ~AbstractSyntaxTree() = default;
    virtual bool operator==(const AbstractSyntaxTree &other) const = 0;
    bool operator!=(const AbstractSyntaxTree &other) const {
        return !(*this == other);
    };
    virtual void compile(Program &program, Segment &segment) const {
        throw std::runtime_error("[" + typeStr + "::compile] Unimplemented method!");
    }
};

struct Node final : public AbstractSyntaxTree {
    Token token;
    explicit Node(Token token);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct BinaryExpression final : public AbstractSyntaxTree {
    AbstractSyntaxTree *left{};
    AbstractSyntaxTree *right{};
    Token op;
    BinaryExpression(AbstractSyntaxTree *left, AbstractSyntaxTree *right, Token op);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct UnaryExpression final : public AbstractSyntaxTree {
    enum class Side {
        LEFT,
        RIGHT,
    } side;
    AbstractSyntaxTree *expression;
    Token op;
    UnaryExpression(AbstractSyntaxTree *expression, Token op, Side side);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct Declaration final : public AbstractSyntaxTree {
    std::optional<AbstractSyntaxTree *> type;
    Node identifier;
    std::optional<AbstractSyntaxTree *> value;
    Declaration(AbstractSyntaxTree *type, Node identifier, AbstractSyntaxTree *value);
    Declaration(AbstractSyntaxTree *type, Node identifier);
    Declaration(Node identifier, AbstractSyntaxTree *value);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ScopedBody : public AbstractSyntaxTree {
    std::vector<AbstractSyntaxTree *> body;
    explicit ScopedBody(const std::vector<AbstractSyntaxTree *> &body);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
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
    void compile(Program &program, Segment &segment) const override;
};

struct TypeCast : public AbstractSyntaxTree {
    AbstractSyntaxTree *expression;
    AbstractSyntaxTree *type;
    TypeCast(AbstractSyntaxTree *expression, AbstractSyntaxTree *type);
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct FunctionCall : public AbstractSyntaxTree {
    Node identifier;
    std::vector<AbstractSyntaxTree *> arguments;
    FunctionCall(Node identifier, const std::vector<AbstractSyntaxTree *> &arguments);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct IfStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *thenBody;
    std::optional<AbstractSyntaxTree *> elseBody;
    IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody, AbstractSyntaxTree *elseBody);
    IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct WhileStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *body;
    WhileStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *body);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ForLoop : public AbstractSyntaxTree {
    AbstractSyntaxTree *initialization;
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *step;
    AbstractSyntaxTree *body;
    ForLoop(AbstractSyntaxTree *initialization, AbstractSyntaxTree *condition, AbstractSyntaxTree *step, AbstractSyntaxTree *body);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

std::vector<AbstractSyntaxTree *> parse(const char *input);
void compile(Program &program, const char *input);
Program compile(const char *input);