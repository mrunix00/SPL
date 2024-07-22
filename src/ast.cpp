#include "ast.h"
#include "utils.h"

#include <utility>

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
        case String: {
            auto string = new StringObject(token.value.size(), (char *) token.value.c_str());
            segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::LoadObject,
                            .params = {.ptr = string},
                    });
        } break;
        case Number: {
            auto type = deduceType(program, segment, (AbstractSyntaxTree *) this);
            return segment.instructions.push_back(emitLoad(type, token));
        }
        case Identifier: {
            return emitLoad(program, segment, token.value);
        }
        case False:
        case True:
            return segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::LoadI32,
                            .params = {.i32 = token.value == "true"},
                    });
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
        emitStore(program, segment, dynamic_cast<Node &>(*left).token.value);
        return;
    }
    auto leftType = deduceType(program, segment, left);
    auto rightType = deduceType(program, segment, right);
    auto finalType = biggestType(leftType, rightType);

    left->compile(program, segment);
    typeCast(segment.instructions, leftType, finalType);
    right->compile(program, segment);
    typeCast(segment.instructions, rightType, finalType);
    switch (op.type) {
        case Plus:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Add, finalType));
        case Minus:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Sub, finalType));
        case Multiply:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Mul, finalType));
        case Divide:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Div, finalType));
        case Modulo:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Mod, finalType));
        case Greater:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Greater, finalType));
        case Less:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Less, finalType));
        case GreaterEqual:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::GreaterEqual, finalType));
        case LessEqual:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::LessEqual, finalType));
        case Equal:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::Equal, finalType));
        case NotEqual:
            return segment.instructions.push_back(getInstructionWithType(GenericInstruction::NotEqual, finalType));
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
    if (!type.has_value()) {
        if (value.has_value()) {
            auto varType = deduceType(program, segment, value.value());
            value.value()->compile(program, segment);
            segment.declare_variable(identifier.token.value, new VariableType(varType));
            emitStore(program, segment, identifier.token.value);
        } else {
            throw std::runtime_error("[Declaration::compile] Cannot deduce the variable type!");
        }
        return;
    }
    switch (type.value()->nodeType) {
        case AbstractSyntaxTree::Type::Node: {
            switch (((Node *) type.value())->token.type) {
                case Str: {
                    value.value()->compile(program, segment);
                    segment.declare_variable(identifier.token.value, new VariableType(VariableType::Object));
                    emitStore(program, segment, identifier.token.value);
                } break;
                case Bool:
                    DECLARE_VAR_CASE(U32, u32, uint32_t)
                    DECLARE_VAR_CASE(I32, i32, int32_t)
                    DECLARE_VAR_CASE(I64, i64, int64_t)
                default:
                    throw std::runtime_error("[Declaration::compile] Unimplemented type handler!");
            }
        } break;
        case AbstractSyntaxTree::Type::FunctionDeclaration: {
            auto functionDeclaration = (FunctionDeclaration *) type.value();
            auto newSegment = Segment{.id = program.segments.size()};
            auto returnType = new VariableType(varTypeConvert(functionDeclaration->returnType));
            auto arguments = std::vector<VariableType *>();
            for (auto arg: functionDeclaration->arguments)
                arguments.push_back(new VariableType(varTypeConvert(arg->type.value())));
            segment.declare_function(identifier.token.value,
                                     new FunctionType(returnType, arguments),
                                     program.segments.size());
            for (auto argument: functionDeclaration->arguments) {
                newSegment.locals[argument->identifier.token.value] = Variable(
                        argument->identifier.token.value,
                        new VariableType(deduceType(program, segment, argument)),
                        newSegment.locals.size(),
                        sizeOfType(varTypeConvert(functionDeclaration->returnType)));
                newSegment.locals_capacity += sizeOfType(deduceType(program, segment, argument));
            }
            value.value()->compile(program, newSegment);
            program.segments.push_back(newSegment);
        } break;
        default:
            throw std::runtime_error("[Declaration::compile] Invalid type!");
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
    auto function = program.find_function(segment, identifier.token.value);
    auto functionType = (FunctionType *) function.type;
    for (int i = 0; i < arguments.size(); i++) {
        auto argument = arguments[i];
        auto definedArgument = functionType->arguments[i];
        argument->compile(program, segment);
        typeCast(segment.instructions,
                 deduceType(program, segment, argument),
                 definedArgument->type);
    }

    segment.instructions.push_back(
            Instruction{
                    .type = Instruction::InstructionType::Call,
                    .params = {.index = function.index},
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
    if (deduceType(program, segment, condition) != VariableType::Bool)
        throw std::runtime_error("[IfStatement::compile] Condition must be a boolean!");
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
    if (expression->nodeType != AbstractSyntaxTree::Type::Node)
        throw std::runtime_error("[UnaryExpression::compile] Invalid expression varType!");
    Node *node = dynamic_cast<Node *>(expression);
    if (node->token.type != Identifier)
        throw std::runtime_error("[UnaryExpression::compile] Invalid expression varType!");

    VariableType *varType;
    if (segment.find_local(node->token.value) != -1) {
        varType = segment.locals[node->token.value].type;
    } else if (program.find_global(node->token.value) != -1) {
        varType = program.segments[0].locals[node->token.value].type;
    } else {
        throw std::runtime_error("[UnaryExpression::compile] Identifier not found: " + node->token.value);
    }

    emitLoad(program, segment, node->token.value);
    switch (op.type) {
        case Increment:
            switch (varType->type) {
                VAR_CASE(Increment, U32)
                VAR_CASE(Increment, I32)
                VAR_CASE(Increment, I64)
                default:
                    throw std::runtime_error("[UnaryExpression::compile] Invalid varType!");
            }
            break;
        case Decrement:
            switch (varType->type) {
                VAR_CASE(Decrement, U32)
                VAR_CASE(Decrement, I32)
                VAR_CASE(Decrement, I64)
                default:
                    throw std::runtime_error("[UnaryExpression::compile] Invalid varType!");
            }
            break;
        default:
            throw std::runtime_error("[UnaryExpression::compile] Invalid operator: " + op.value);
    }
    emitStore(program, segment, node->token.value);
}
bool UnaryExpression::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherUnaryExpression = dynamic_cast<const UnaryExpression &>(other);
    return *expression == *otherUnaryExpression.expression &&
           op == otherUnaryExpression.op &&
           side == otherUnaryExpression.side;
}

ForLoop::ForLoop(AbstractSyntaxTree *initialization, AbstractSyntaxTree *condition, AbstractSyntaxTree *step, AbstractSyntaxTree *body)
    : initialization(initialization), condition(condition), step(step), body(body) {
    nodeType = Type::ForLoop;
    typeStr = "ForLoop";
}
bool ForLoop::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherForLoop = dynamic_cast<const ForLoop &>(other);
    if ((initialization == nullptr || otherForLoop.initialization == nullptr) &&
        initialization != otherForLoop.initialization)
        return false;
    if ((condition == nullptr || otherForLoop.condition == nullptr) &&
        condition != otherForLoop.condition)
        return false;
    if ((step == nullptr || otherForLoop.step == nullptr) &&
        step != otherForLoop.step)
        return false;
    if ((body == nullptr || otherForLoop.body == nullptr) &&
        body != otherForLoop.body)
        return false;
    return *initialization == *otherForLoop.initialization &&
           *condition == *otherForLoop.condition &&
           *step == *otherForLoop.step &&
           *body == *otherForLoop.body;
}
void ForLoop::compile(Program &program, Segment &segment) const {
    size_t condition_index, jump_index;
    initialization->compile(program, segment);
    condition_index = segment.instructions.size();
    condition->compile(program, segment);
    jump_index = segment.instructions.size();
    segment.instructions.push_back({Instruction::InstructionType::JumpIfFalse});
    body->compile(program, segment);
    step->compile(program, segment);
    segment.instructions.push_back({
            .type = Instruction::InstructionType::Jump,
            .params = {.index = condition_index},
    });
    segment.instructions[jump_index].params = {.index = segment.instructions.size()};
}

void compile(Program &program, const char *input) {
    auto ast = parse(input);
    for (auto &node: ast) {
        node->compile(program, program.segments[0]);
    }
}
Program compile(const char *input) {
    Program program;
    compile(program, input);
    return program;
}
