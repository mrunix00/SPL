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
void Node::compile(Program &program, Segment &segment) const {
    switch (token.type) {
        case Number:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::LoadI32,
                            .params = {.i32 = std::stoi(token.value)},
                    });
            break;
        default:
            throw std::runtime_error("Invalid operator: " + token.value);
    }
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
void BinaryExpression::compile(Program &program, Segment &segment) const {
left->compile(program, segment);
    right->compile(program, segment);
    switch (op.type) {
        case Plus:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::AddI32,
                    });
            break;
        case Minus:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::SubI32,
                    });
            break;
        case Multiply:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::MulI32,
                    });
            break;
        case Divide:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::DivI32,
                    });
            break;
        case Modulo:
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::ModI32,
                    });
            break;
        default:
            throw std::runtime_error("Invalid operator: " + op.value);
    }
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
void Declaration::compile(Program &program, Segment &segment) const {
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
void ScopedBody::compile(Program &program, Segment &segment) const {
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
void FunctionDeclaration::compile(Program &program, Segment &segment) const {
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
void ReturnStatement::compile(Program &program, Segment &segment) const {
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
void TypeCast::compile(Program &program, Segment &segment) const {
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
void FunctionCall::compile(Program &program, Segment &segment) const {
}

IfStatement::IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody, AbstractSyntaxTree *elseBody)
    : condition(condition), thenBody(thenBody), elseBody(std::make_optional<AbstractSyntaxTree *>(elseBody)) {
    nodeType = Type::IfStatement;
    assert(condition != nullptr, "Condition can't be null!");
    assert(thenBody != nullptr, "Then body can't be null!");
}
IfStatement::IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody)
    : condition(condition), thenBody(thenBody) {
    nodeType = Type::IfStatement;
    assert(condition != nullptr, "Condition can't be null!");
    assert(thenBody != nullptr, "Then body can't be null!");
}
bool IfStatement::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherIfStatement = dynamic_cast<const IfStatement &>(other);

    if (elseBody.has_value() && otherIfStatement.elseBody.has_value() &&
        !(*elseBody.value() == *otherIfStatement.elseBody.value()))
        return false;

    return *condition == *otherIfStatement.condition &&
           *thenBody == *otherIfStatement.thenBody &&
           elseBody.has_value() == otherIfStatement.elseBody.has_value();
}
void IfStatement::compile(Program &program, Segment &segment) const {
}

Program compile(const char *input) {
    Program program;
    auto ast = parse(input);
    for (auto &node : ast) {
        node->compile(program, program.segments[0]);
    }
    return program;
}