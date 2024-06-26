#include "ast.h"

#include <stdexcept>
#include <utility>

static inline void assert(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error("Assertion failed: " + std::string(message));
}

Node::Node(Token token) : token(std::move(token)) {
    nodeType = Type::Node;
    typeStr = "Node";
}
bool Node::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    return token == dynamic_cast<const Node &>(other).token;
}
void Node::compile(Program &program, Segment &segment) const {
    switch (token.type) {
        case Number: {
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::LoadI32,
                            .params = {.i32 = std::stoi(token.value)},
                    });
        } break;
        case Identifier: {
            if (segment.find_local(token.value) != -1) {
                segment.instructions.push_back(
                        Instruction{
                                .type = Instruction::InstructionType::LoadLocalI32,
                                .params = {.index = segment.find_local(token.value)},
                        });
            } else if (program.find_global(token.value) != -1) {
                segment.instructions.push_back(
                        Instruction{
                                .type = Instruction::InstructionType::LoadGlobalI32,
                                .params = {.index = program.find_global(token.value)},
                        });
            } else {
                throw std::runtime_error("[Node::compile] Identifier not found: " + token.value);
            }
        } break;
        default:
            throw std::runtime_error("[Node::compile] This should not be accessed!");
    }
}

BinaryExpression::BinaryExpression(AbstractSyntaxTree *left, AbstractSyntaxTree *right, Token op)
    : left(left), right(right), op(std::move(op)) {
    nodeType = Type::BinaryExpression;
    typeStr = "BinaryExpression";
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
    if (op.type == Assign) {
        right->compile(program, segment);
        if (segment.find_local(dynamic_cast<Node &>(*left).token.value)) {
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::StoreLocalI32,
                            .params = {.index = segment.find_local(dynamic_cast<Node &>(*left).token.value)},
                    });
        } else if (program.find_global(dynamic_cast<Node &>(*left).token.value) != -1) {
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::StoreGlobalI32,
                            .params = {.index = program.find_global(dynamic_cast<Node &>(*left).token.value)},
                    });
        }
        return;
    }

    if (right->nodeType == AbstractSyntaxTree::Type::Node &&
        left->nodeType == AbstractSyntaxTree::Type::Node &&
        ((Node *) left)->token.type == Identifier &&
        ((Node *) right)->token.type == Number) {
        Instruction instruction;
        bool isLocal = segment.find_local(((Node *) left)->token.value) != -1;
        if (isLocal) {
            instruction.params.ri = {
                    .index = segment.find_local(((Node *) left)->token.value),
                    .i32 = std::stoi(((Node *) right)->token.value),
            };
        } else if (program.find_global(((Node *) left)->token.value) != -1) {
            instruction.params.ri = {
                    .index = program.find_global(((Node *) left)->token.value),
                    .i32 = std::stoi(((Node *) right)->token.value),
            };
        } else {
            throw std::runtime_error("[BinaryExpression::compile] Identifier not found: " + ((Node *) left)->token.value);
        }

        switch (op.type) {
            case Plus:
                instruction.type = isLocal ? Instruction::InstructionType::AddI32_RI
                                           : Instruction::InstructionType::AddI32_GI;
                break;
            case Minus:
                instruction.type = isLocal ? Instruction::InstructionType::SubI32_RI
                                           : Instruction::InstructionType::SubI32_GI;
                break;
            case Multiply:
                instruction.type = isLocal ? Instruction::InstructionType::MulI32_RI
                                           : Instruction::InstructionType::MulI32_GI;
                break;
            case Divide:
                instruction.type = isLocal ? Instruction::InstructionType::DivI32_RI
                                           : Instruction::InstructionType::DivI32_GI;
                break;
            case Modulo:
                instruction.type = isLocal ? Instruction::InstructionType::ModI32_RI
                                           : Instruction::InstructionType::ModI32_GI;
                break;
            case Greater:
                instruction.type = isLocal ? Instruction::InstructionType::GreaterI32_RI
                                           : Instruction::InstructionType::GreaterI32_GI;
                break;
            case Less:
                instruction.type = isLocal ? Instruction::InstructionType::LessI32_RI
                                           : Instruction::InstructionType::LessI32_GI;
                break;
            default:
                throw std::runtime_error("[BinaryExpression::compile] Invalid operator: " + op.value);
        }
        segment.instructions.push_back(instruction);
        return;
    }

    left->compile(program, segment);
    if (right->nodeType == AbstractSyntaxTree::Type::Node &&
        ((Node *) right)->token.value == "1" &&
        left->nodeType == AbstractSyntaxTree::Type::Node &&
        ((Node *) left)->token.type == Identifier) {
        switch (op.type) {
            case Plus:
                segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::IncrementI32});
                return;
            case Minus:
                segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::DecrementI32});
                return;
            case Multiply:
            case Divide:
                return;
        }
    }

    right->compile(program, segment);
    switch (op.type) {
        case Plus:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::AddI32});
            break;
        case Minus:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::SubI32});
            break;
        case Multiply:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::MulI32});
            break;
        case Divide:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::DivI32});
            break;
        case Modulo:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::ModI32});
            break;
        case Greater:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::GreaterI32});
            break;
        case Less:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::LessI32});
            break;
        case GreaterEqual:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::GreaterEqualI32});
            break;
        case LessEqual:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::LessEqualI32});
            break;
        case Equal:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::EqualI32});
            break;
        case NotEqual:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::NotEqualI32});
            break;
        default:
            throw std::runtime_error("[BinaryExpression::compile] Invalid operator: " + op.value);
    }
}

Declaration::Declaration(AbstractSyntaxTree *type, Node identifier, AbstractSyntaxTree *value)
    : type(type), identifier(std::move(identifier)), value(std::make_optional<AbstractSyntaxTree *>(value)) {
    nodeType = Type::Declaration;
    typeStr = "Declaration";
    assert(value != nullptr, "Initialization can't be null!");
    assert(type != nullptr, "Type can't be null!");
}
Declaration::Declaration(AbstractSyntaxTree *type, Node identifier)
    : type(type), identifier(std::move(identifier)) {
    nodeType = Type::Declaration;
    typeStr = "Declaration";
    assert(type != nullptr, "Type can't be null!");
}
Declaration::Declaration(Node identifier, AbstractSyntaxTree *value)
    : identifier(std::move(identifier)), value(std::make_optional<AbstractSyntaxTree *>(value)) {
    nodeType = Type::Declaration;
    typeStr = "Declaration";
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
    if (!type.has_value())
        throw std::runtime_error("[Declaration::compile] Type deduction is not implemented!");

    switch (type.value()->nodeType) {
        case AbstractSyntaxTree::Type::Node: {
            switch (((Node *) type.value())->token.type) {
                case I32: {
                    value.value()->compile(program, segment);
                    segment.instructions.push_back({
                            .type = segment.id == 0
                                            ? Instruction::InstructionType::StoreGlobalI32
                                            : Instruction::InstructionType::StoreLocalI32,
                            .params = {.index = segment.locals.size()},
                    });
                    segment.declare_variable(identifier.token.value, Variable::Type::I32);
                } break;
                default:
                    throw std::runtime_error("[Declaration::compile] Unimplemented type handler!");
            }
        } break;
        case AbstractSyntaxTree::Type::FunctionDeclaration: {
            auto functionDeclaration = (FunctionDeclaration *) type.value();
            auto newSegment = Segment{.id = program.segments.size()};
            segment.declare_function(identifier.token.value, program.segments.size());
            segment.declare_variable(identifier.token.value, Variable::Type::Function);
            for (auto argument: functionDeclaration->arguments) {
                newSegment.locals[argument->identifier.token.value] = {
                        .name = argument->identifier.token.value,
                        .type = Variable::Type::I32,// TODO: Handle all variable types
                        .index = newSegment.locals.size(),
                };
            }
            value.value()->compile(program, newSegment);
            program.segments.push_back(newSegment);
        } break;
        default:
            throw std::runtime_error("[Declaration::compile] Unimplemented type handler!");
    }
}

ScopedBody::ScopedBody(const std::vector<AbstractSyntaxTree *> &body)
    : body(body) {
    nodeType = Type::ScopedBody;
    typeStr = "ScopedBody";
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
    for (auto &node: body)
        node->compile(program, segment);
}

FunctionDeclaration::FunctionDeclaration(AbstractSyntaxTree *returnType, const std::vector<Declaration *> &arguments)
    : returnType(returnType), arguments(arguments) {
    nodeType = Type::FunctionDeclaration;
    typeStr = "FunctionDeclaration";
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
    typeStr = "ReturnStatement";
    assert(expression != nullptr, "Return expression can't be null!");
}
bool ReturnStatement::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    return *expression == *dynamic_cast<const ReturnStatement &>(other).expression;
}
void ReturnStatement::compile(Program &program, Segment &segment) const {
    expression->compile(program, segment);
    segment.instructions.push_back(
            Instruction{
                    .type = Instruction::InstructionType::Return,
            });
}

TypeCast::TypeCast(AbstractSyntaxTree *expression, AbstractSyntaxTree *type)
    : expression(expression), type(type) {
    nodeType = Type::TypeCast;
    typeStr = "TypeCast";
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
    typeStr = "FunctionCall";
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
    for (auto &argument: arguments) {
        argument->compile(program, segment);
    }

    segment.instructions.push_back(
            Instruction{
                    .type = Instruction::InstructionType::Call,
                    .params = {.index = program.find_function(segment, identifier.token.value)},
            });
}

IfStatement::IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody, AbstractSyntaxTree *elseBody)
    : condition(condition), thenBody(thenBody), elseBody(std::make_optional<AbstractSyntaxTree *>(elseBody)) {
    nodeType = Type::IfStatement;
    typeStr = "IfStatement";
    assert(condition != nullptr, "Condition can't be null!");
    assert(thenBody != nullptr, "Then body can't be null!");
}
IfStatement::IfStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *thenBody)
    : condition(condition), thenBody(thenBody) {
    nodeType = Type::IfStatement;
    typeStr = "IfStatement";
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
    condition->compile(program, segment);
    size_t jumpIndex = segment.instructions.size();
    segment.instructions.push_back(
            Instruction{.type = Instruction::InstructionType::JumpIfFalse});
    thenBody->compile(program, segment);
    segment.instructions[jumpIndex].params.index = segment.instructions.size() + elseBody.has_value();
    if (elseBody.has_value()) {
        jumpIndex = segment.instructions.size();
        segment.instructions.push_back(
                Instruction{.type = Instruction::InstructionType::Jump});
        elseBody.value()->compile(program, segment);
        segment.instructions[jumpIndex].params.index = segment.instructions.size() + 1;
    }
}

WhileStatement::WhileStatement(AbstractSyntaxTree *condition, AbstractSyntaxTree *body)
    : condition(condition), body(body) {
    nodeType = Type::WhileStatement;
    typeStr = "WhileStatement";
    assert(condition != nullptr, "Condition can't be null!");
    assert(body != nullptr, "Body can't be null!");
}
bool WhileStatement::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherWhileStatement = dynamic_cast<const WhileStatement &>(other);
    return *condition == *otherWhileStatement.condition && *body == *otherWhileStatement.body;
}
void WhileStatement::compile(Program &program, Segment &segment) const {
    size_t jumpIndex = segment.instructions.size();
    condition->compile(program, segment);
    size_t bodyIndex = segment.instructions.size();
    segment.instructions.push_back(
            Instruction{.type = Instruction::InstructionType::JumpIfFalse});
    body->compile(program, segment);
    segment.instructions.push_back(
            Instruction{.type = Instruction::InstructionType::Jump, .params = {.index = jumpIndex}});
    segment.instructions[bodyIndex].params.index = segment.instructions.size();
}

UnaryExpression::UnaryExpression(AbstractSyntaxTree *expression, Token op, UnaryExpression::Side side)
    : expression(expression), op(std::move(op)), side(side) {
    nodeType = Type::UnaryExpression;
    typeStr = "UnaryExpression";
    assert(expression != nullptr, "Expression can't be null!");
}
void UnaryExpression::compile(Program &program, Segment &segment) const {
    expression->compile(program, segment);
    size_t index = segment.instructions.size() - 1;
    switch (op.type) {
        case Increment:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::IncrementI32});
            break;
        case Decrement:
            segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::DecrementI32});
            break;
        default:
            throw std::runtime_error("[UnaryExpression::compile] Invalid operator: " + op.value);
    }
    if (segment.instructions[index].type == Instruction::InstructionType::LoadLocalI32)
        segment.instructions.push_back(
                Instruction{
                        .type = Instruction::InstructionType::StoreLocalI32,
                        .params = {.index = segment.find_local(dynamic_cast<Node &>(*expression).token.value)},
                });
    else if (segment.instructions[index].type == Instruction::InstructionType::LoadGlobalI32)
        segment.instructions.push_back(
                Instruction{
                        .type = Instruction::InstructionType::StoreGlobalI32,
                        .params = {.index = program.find_global(dynamic_cast<Node &>(*expression).token.value)},
                });
    else
        throw std::runtime_error("[UnaryExpression::compile] Invalid expression type!");
}
bool UnaryExpression::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherUnaryExpression = dynamic_cast<const UnaryExpression &>(other);
    return *expression == *otherUnaryExpression.expression &&
           op == otherUnaryExpression.op &&
           side == otherUnaryExpression.side;
}

Program compile(const char *input) {
    Program program;
    auto ast = parse(input);
    for (auto &node: ast) {
        node->compile(program, program.segments[0]);
    }
    return program;
}
