#include "ast.h"

#include <stdexcept>
#include <utility>

static inline void assert(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error("Assertion failed: " + std::string(message));
}

Node::Node(Token token) : token(std::move(token)) {
    nodeType = Type::Node;
}
bool Node::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    return token == dynamic_cast<const Node &>(other).token;
}

BinaryExpression::BinaryExpression(AbstractSyntaxTree *left, AbstractSyntaxTree *right, Token op)
    : left(left), right(right), op(std::move(op)) {
    nodeType = Type::BinaryExpression;
    assert(left != nullptr, "Left operand can't be null!");
    assert(right != nullptr, "Right operand can't be null!");
}
bool BinaryExpression::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherBinaryExpression = dynamic_cast<const BinaryExpression &>(other);
    return *left == *otherBinaryExpression.left &&
           *right == *otherBinaryExpression.right &&
           op == otherBinaryExpression.op;
}

Declaration::Declaration(AbstractSyntaxTree *type, Node identifier, AbstractSyntaxTree *value)
    : type(type), identifier(std::move(identifier)), value(std::make_optional<AbstractSyntaxTree *>(value)) {
    nodeType = Type::Declaration;
    assert(value != nullptr, "Initialization can't be null!");
    assert(type != nullptr, "Type can't be null!");
}
Declaration::Declaration(AbstractSyntaxTree *type, Node identifier)
    : type(type), identifier(std::move(identifier)) {
    nodeType = Type::Declaration;
    assert(type != nullptr, "Type can't be null!");
}
Declaration::Declaration(Node identifier, AbstractSyntaxTree *value)
    : identifier(std::move(identifier)), value(std::make_optional<AbstractSyntaxTree *>(value)) {
    nodeType = Type::Declaration;
    assert(value != nullptr, "Initialization can't be null!");
}
bool Declaration::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherDeclaration = dynamic_cast<const Declaration &>(other);
    if (value.has_value() && otherDeclaration.value.has_value() &&
        !(*value.value() == *otherDeclaration.value.value()))
        return false;
    if (type.has_value() && otherDeclaration.type.has_value() &&
        !(*type.value() == *otherDeclaration.type.value()))
        return false;
    return type.has_value() == otherDeclaration.type.has_value() &&
           identifier == otherDeclaration.identifier &&
           value.has_value() == otherDeclaration.value.has_value();
}

ScopedBody::ScopedBody(const std::vector<AbstractSyntaxTree *> &body)
    : body(body) {
    nodeType = Type::ScopedBody;
}
bool ScopedBody::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherScopedBody = dynamic_cast<const ScopedBody &>(other);

    if (body.size() != otherScopedBody.body.size()) return false;
    for (size_t i = 0; i < body.size(); i++) {
        if (*body[i] != *otherScopedBody.body[i]) {
            return false;
        }
    }

    return true;
}

FunctionDeclaration::FunctionDeclaration(AbstractSyntaxTree *returnType, const std::vector<Declaration *> &arguments)
    : returnType(returnType), arguments(arguments) {
    nodeType = Type::FunctionDeclaration;
    assert(returnType != nullptr, "Return type can't be null!");
}
bool FunctionDeclaration::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherFunctionDeclaration = dynamic_cast<const FunctionDeclaration &>(other);

    if (*returnType != *otherFunctionDeclaration.returnType) return false;
    if (arguments.size() != otherFunctionDeclaration.arguments.size()) return false;
    for (size_t i = 0; i < arguments.size(); i++) {
        if (!(*arguments[i] == *otherFunctionDeclaration.arguments[i])) {
            return false;
        }
    }

    return true;
}

ReturnStatement::ReturnStatement(AbstractSyntaxTree *expression)
    : expression(expression) {
    nodeType = Type::ReturnStatement;
    assert(expression != nullptr, "Return expression can't be null!");
}
bool ReturnStatement::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    return *expression == *dynamic_cast<const ReturnStatement &>(other).expression;
}

TypeCast::TypeCast(AbstractSyntaxTree *expression, AbstractSyntaxTree *type)
    : expression(expression), type(type) {
    nodeType = Type::TypeCast;
    assert(expression != nullptr, "Expression can't be null!");
    assert(type != nullptr, "Type can't be null!");
}
bool TypeCast::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherTypeCast = dynamic_cast<const TypeCast &>(other);
    return *expression == *otherTypeCast.expression && *type == *otherTypeCast.type;
}

FunctionCall::FunctionCall(Node identifier, const std::vector<AbstractSyntaxTree *> &arguments)
    : identifier(std::move(identifier)), arguments(arguments) {
    nodeType = Type::FunctionCall;
}
bool FunctionCall::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherFunctionCall = dynamic_cast<const FunctionCall &>(other);

    if (arguments.size() != otherFunctionCall.arguments.size()) return false;
    for (size_t i = 0; i < arguments.size(); i++) {
        if (!(*arguments[i] == *otherFunctionCall.arguments[i])) {
            return false;
        }
    }

    return identifier == otherFunctionCall.identifier;
}
