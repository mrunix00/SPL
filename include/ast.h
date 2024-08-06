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
        List,
        ArrayType,
        ArrayAccess,
        ImportStatement,
        ExportStatement,
        TernaryExpression,
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
    ~BinaryExpression() override {
        delete left;
        delete right;
    }
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
    ~UnaryExpression() override {
        delete expression;
    }
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
    ~Declaration() override {
        if (type.has_value()) delete type.value();
        if (value.has_value()) delete value.value();
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ScopedBody : public AbstractSyntaxTree {
    std::vector<AbstractSyntaxTree *> body;
    explicit ScopedBody(const std::vector<AbstractSyntaxTree *> &body);
    ~ScopedBody() override {
        for (auto &node : body)
            delete node;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct FunctionDeclaration : public AbstractSyntaxTree {
    AbstractSyntaxTree *returnType;
    std::vector<Declaration *> arguments;
    FunctionDeclaration(AbstractSyntaxTree *returnType, const std::vector<Declaration *> &arguments);
    ~FunctionDeclaration() override {
        delete returnType;
        for (auto &arg : arguments)
            delete arg;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct ReturnStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *expression;
    explicit ReturnStatement(AbstractSyntaxTree *expression);
    ~ReturnStatement() override {
        delete expression;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct TypeCast : public AbstractSyntaxTree {
    AbstractSyntaxTree *expression;
    AbstractSyntaxTree *type;
    TypeCast(AbstractSyntaxTree *expression, AbstractSyntaxTree *type);
    ~TypeCast() override {
        delete expression;
        delete type;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct FunctionCall : public AbstractSyntaxTree {
    Node identifier;
    std::vector<AbstractSyntaxTree *> arguments;
    FunctionCall(Node identifier, const std::vector<AbstractSyntaxTree *> &arguments);
    ~FunctionCall() override {
        for (auto &arg : arguments)
            delete arg;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct IfStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *thenBody;
    std::optional<AbstractSyntaxTree *> elseBody;
    IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody, AbstractSyntaxTree *elseBody);
    IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody);
    ~IfStatement() override {
        delete condition;
        delete thenBody;
        if (elseBody.has_value()) delete elseBody.value();
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct WhileStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *body;
    WhileStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *body);
    ~WhileStatement() override {
        delete condition;
        delete body;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ForLoop : public AbstractSyntaxTree {
    AbstractSyntaxTree *initialization;
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *step;
    AbstractSyntaxTree *body;
    ForLoop(AbstractSyntaxTree *initialization, AbstractSyntaxTree *condition, AbstractSyntaxTree *step, AbstractSyntaxTree *body);
    ~ForLoop() override {
        delete initialization;
        delete condition;
        delete step;
        delete body;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct List : public AbstractSyntaxTree {
    std::vector<AbstractSyntaxTree *> elements;
    explicit List(const std::vector<AbstractSyntaxTree *> &elements);
    ~List() override {
        for (auto &element : elements)
            delete element;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ArrayType : public AbstractSyntaxTree {
    AbstractSyntaxTree *type;
    explicit ArrayType(AbstractSyntaxTree *type);
    ~ArrayType() override {
        delete type;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
};

struct ArrayAccess : public AbstractSyntaxTree {
    Node identifier;
    AbstractSyntaxTree *index;
    ArrayAccess(Node identifier, AbstractSyntaxTree *index);
    ~ArrayAccess() override {
        delete index;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ImportStatement : public AbstractSyntaxTree {
    std::string path;
    explicit ImportStatement(std::string  path);
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct ExportStatement : public AbstractSyntaxTree {
    AbstractSyntaxTree *stm;
    explicit ExportStatement(AbstractSyntaxTree *stm);
    ~ExportStatement() override {
        delete stm;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

struct TernaryExpression : public AbstractSyntaxTree {
    AbstractSyntaxTree *condition;
    AbstractSyntaxTree *thenCase;
    AbstractSyntaxTree *elseCase;
    TernaryExpression(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenCase, AbstractSyntaxTree *elseCase);
    ~TernaryExpression() override {
        delete condition;
        delete thenCase;
        delete elseCase;
    }
    bool operator==(const AbstractSyntaxTree &other) const override;
    void compile(Program &program, Segment &segment) const override;
};

std::vector<AbstractSyntaxTree *> parse(const char *input);
void compile(Program &program, const char *input);
Program compile(const char *input);
