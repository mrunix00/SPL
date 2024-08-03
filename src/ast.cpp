#include "ast.h"
#include "utils.h"

#include <fstream>
#include <sstream>
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
            return segment.instructions.push_back(emitLoad(type->type, token));
        }
        case Identifier: {
            return emitLoad(program, segment, token.value);
        }
        case False:
        case True:
            return segment.instructions.push_back(
                    Instruction{
                            .type = Instruction::InstructionType::LoadI64,
                            .params = {.i64 = token.value == "true"},
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
    if ((op.type == IncrementAssign || op.type == DecrementAssign) &&
        left->nodeType == AbstractSyntaxTree::Type::Node) {
        auto node = dynamic_cast<Node *>(left);
        if (node->token.type != Identifier)
            throw std::runtime_error("[BinaryExpression::compile] Invalid expression varType!");
        auto varType = segment.find_local(node->token.value) != -1 ? segment.locals[node->token.value] : program.segments[0].locals[node->token.value];
        switch (op.type) {
            case IncrementAssign:
                switch (varType.type->type) {
                    case VariableType::Type::I64:
                        emitLoad(program, segment, node->token.value);
                        right->compile(program, segment);
                        segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::AddI64});
                        break;
                    case VariableType::Array: {
                        right->compile(program, segment);
                        left->compile(program, segment);
                        segment.instructions.push_back({.type = Instruction::AppendToArray});
                    } break;
                    default:
                        throw std::runtime_error("[BinaryExpression::compile] Invalid varType!");
                }
                break;
            case DecrementAssign:
                switch (varType.type->type) {
                    case VariableType::Type::I64:
                        emitLoad(program, segment, node->token.value);
                        right->compile(program, segment);
                        segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::SubI64});
                        break;
                    default:
                        throw std::runtime_error("[BinaryExpression::compile] Invalid varType!");
                }
                break;
            default:
                throw std::runtime_error("[BinaryExpression::compile] Invalid operator: " + op.value);
        }
        emitStore(program, segment, node->token.value);
        return;
    }

    auto leftType = deduceType(program, segment, left);
    auto rightType = deduceType(program, segment, right);
    auto finalType = biggestType(leftType->type, rightType->type);

    left->compile(program, segment);
    typeCast(segment.instructions, leftType->type, finalType);
    right->compile(program, segment);
    typeCast(segment.instructions, rightType->type, finalType);
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
            segment.declare_variable(identifier.token.value, varType);
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
                case Int: {
                    if (!value.has_value()) {
                        segment.instructions.push_back({
                                .type = Instruction::InstructionType::LoadI64,
                                .params = {.i64 = 0},
                        });
                    } else if (((Node *) value.value())->token.type == Number) {
                        segment.instructions.push_back({
                                .type = Instruction::InstructionType::LoadI64,
                                .params = {.i64 = convert<int64_t>(((Node *) value.value())->token.value)},
                        });
                    } else {
                        ((Node *) value.value())->compile(program, segment);
                    }
                    segment.declare_variable(identifier.token.value, new VariableType(VariableType::Type::I64));
                    segment.instructions.push_back({
                            .type = segment.id == 0 ? Instruction::InstructionType::StoreGlobalI64 : Instruction::InstructionType::StoreLocalI64,
                            .params = {.index = segment.find_local(identifier.token.value)},
                    });
                } break;
                default:
                    throw std::runtime_error("[Declaration::compile] Unimplemented type handler!");
            }
        } break;
        case AbstractSyntaxTree::Type::FunctionDeclaration: {
            auto functionDeclaration = (FunctionDeclaration *) type.value();
            auto newSegment = Segment{.id = program.segments.size()};
            auto returnType = varTypeConvert(functionDeclaration->returnType);
            auto arguments = std::vector<VariableType *>();
            newSegment.returnType = returnType;
            for (auto arg: functionDeclaration->arguments)
                arguments.push_back(varTypeConvert(arg->type.value()));
            segment.declare_function(identifier.token.value,
                                     new FunctionType(returnType, arguments),
                                     program.segments.size());
            for (auto argument: functionDeclaration->arguments) {
                auto argType = deduceType(program, segment, argument);
                if (argType->type == VariableType::Object ||
                    argType->type == VariableType::Array) {
                    newSegment.number_of_arg_ptr++;
                } else {
                    newSegment.number_of_args++;
                }
                newSegment.declare_variable(argument->identifier.token.value, argType);
            }
            value.value()->compile(program, newSegment);
            if (returnType->type == VariableType::Type::Void &&
                newSegment.instructions.back().type != Instruction::InstructionType::Return) {
                newSegment.instructions.push_back({
                        .type = Instruction::InstructionType::Return,
                });
            }
            program.segments.push_back(newSegment);
        } break;
        case AbstractSyntaxTree::Type::ArrayType: {
            if (value.has_value()) {
                value.value()->compile(program, segment);
                segment.declare_variable(
                        identifier.token.value,
                        varTypeConvert(type.value()));
                segment.instructions.push_back({
                        .type = segment.id == 0 ? Instruction::InstructionType::StoreGlobalObject : Instruction::InstructionType::StoreLocalObject,
                        .params = {.index = segment.find_local(identifier.token.value)},
                });
            }
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
    if (expression != nullptr) {
        expression->compile(program, segment);
        auto type = deduceType(program, segment, expression);
        if (type->type != segment.returnType->type)
            typeCast(segment.instructions, type->type, segment.returnType->type);
    }
    if (expression == nullptr && segment.returnType->type != VariableType::Type::Void)
        throw std::runtime_error("[ReturnStatement::compile] Return type mismatch!");
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
    if (identifier.token.value == "native") {
        arguments.front()->compile(program, segment);
        return segment.instructions.push_back({.type = Instruction::LoadLib});
    }
    VariableType *varType = nullptr;
    if (segment.find_local(identifier.token.value) != -1) {
        varType = segment.locals[identifier.token.value].type;
    } else if (segment.find_local(identifier.token.value) != -1) {
        varType = program.segments.front().locals[identifier.token.value].type;
    }
    if (varType != nullptr && varType->type == VariableType::NativeLib) {
        std::vector<VariableType *> funcArgs;
        auto argName = (Node *) arguments[0];
        auto argList = (List *) arguments[1];
        for (auto arg: argList->elements) {
            // TODO
        }
        segment.instructions.push_back({
                .type = Instruction::LoadObject,
                .params = {.ptr = new DynamicFunctionObject(argName->token.value, funcArgs)},
        });
        emitLoad(program, segment, identifier.token.value);
        segment.instructions.push_back({.type = Instruction::InstructionType::CallNative});
        return;
    }
    auto function = program.find_function(segment, identifier.token.value);
    auto functionType = (FunctionType *) function.type;
    for (int i = 0; i < arguments.size(); i++) {
        auto argument = arguments[i];
        auto definedArgument = functionType->arguments[i];
        argument->compile(program, segment);
        typeCast(segment.instructions,
                 deduceType(program, segment, argument)->type,
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
    if (deduceType(program, segment, condition)->type != VariableType::Bool)
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

    switch (op.type) {
        case Increment:
            switch (varType->type) {
                case VariableType::Type::I64:
                    emitLoad(program, segment, node->token.value);
                    segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::IncrementI64});
                    break;
                default:
                    throw std::runtime_error("[UnaryExpression::compile] Invalid varType!");
            }
            break;
        case Decrement:
            switch (varType->type) {
                case VariableType::Type::I64:
                    emitLoad(program, segment, node->token.value);
                    segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::DecrementI64});
                    break;
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

List::List(const std::vector<AbstractSyntaxTree *> &elements)
    : elements(elements) {
    nodeType = AbstractSyntaxTree::Type::List;
    typeStr = "List";
}
bool List::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherForLoop = dynamic_cast<const List &>(other);
    if (elements.size() != otherForLoop.elements.size()) return false;
    for (size_t i = 0; i < elements.size(); i++) {
        if (!(*elements[i] == *otherForLoop.elements[i])) {
            return false;
        }
    }
    return true;
}
void List::compile(Program &program, Segment &segment) const {
    for (auto element: elements)
        element->compile(program, segment);
    segment.instructions.push_back({
            .type = Instruction::MakeArray,
            .params = {.index = elements.size()},
    });
}

ArrayType::ArrayType(AbstractSyntaxTree *type)
    : type(type) {
    nodeType = AbstractSyntaxTree::Type::ArrayType;
    typeStr = "ArrayType";
}
bool ArrayType::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherArrayType = dynamic_cast<const ArrayType &>(other);
    return *type == *otherArrayType.type;
}

ArrayAccess::ArrayAccess(Node identifier, AbstractSyntaxTree *index)
    : identifier(std::move(identifier)), index(index) {
    nodeType = AbstractSyntaxTree::Type::ArrayAccess;
    typeStr = "ArrayAccess";
}
bool ArrayAccess::operator==(const AbstractSyntaxTree &other) const {
    if (other.nodeType != nodeType) return false;
    auto &otherArrayAccess = dynamic_cast<const ArrayAccess &>(other);
    return identifier == otherArrayAccess.identifier &&
           *index == *otherArrayAccess.index;
}
void ArrayAccess::compile(Program &program, Segment &segment) const {
    bool isLocal;
    if (segment.find_local(identifier.token.value) != -1)
        isLocal = true;
    else if (program.find_global(identifier.token.value) != -1)
        isLocal = false;
    else
        throw std::runtime_error("[ArrayAccess::compile] Identifier not found: " + identifier.token.value);
    auto varIndex = isLocal ? segment.find_local(identifier.token.value) : program.find_global(identifier.token.value);
    index->compile(program, segment);
    segment.instructions.push_back({
            .type = isLocal ? Instruction::LoadFromLocalArray : Instruction::LoadFromGlobalArray,
            .params = {.index = varIndex},
    });
}

ImportStatement::ImportStatement(std::string path)
    : path(std::move(path)) {
    nodeType = AbstractSyntaxTree::Type::ImportStatement;
    typeStr = "ImportStatement";
}
bool ImportStatement::operator==(const AbstractSyntaxTree &other) const {
    if (nodeType != other.nodeType) return false;
    auto &otherImportStatement = dynamic_cast<const ImportStatement &>(other);
    return otherImportStatement.path == path;
}
void ImportStatement::compile(Program &program, Segment &segment) const {
    std::ifstream importedFile(path);
    if (!importedFile.is_open()) {
        throw std::runtime_error("Unable to open file: " + path);
    }
    std::stringstream fileContent;
    fileContent << importedFile.rdbuf();
    auto statements = parse(fileContent.str().c_str());
    for (auto stm: statements) {
        if (stm->nodeType == AbstractSyntaxTree::Type::ExportStatement)
            stm->compile(program, segment);
        delete stm;
    }
}

ExportStatement::ExportStatement(AbstractSyntaxTree *stm)
    : stm(stm) {
    nodeType = AbstractSyntaxTree::Type::ExportStatement;
    typeStr = "ExportStatement";
    assert(stm->nodeType == AbstractSyntaxTree::Type::Declaration,
           "[ExportStatement]: Only declarations can be exported!");
}
bool ExportStatement::operator==(const AbstractSyntaxTree &other) const {
    if (nodeType != other.nodeType) return false;
    auto &otherExportStatement = dynamic_cast<const ExportStatement &>(other);
    return *otherExportStatement.stm == *stm;
}
void ExportStatement::compile(Program &program, Segment &segment) const {
    stm->compile(program, segment);
}

void compile(Program &program, const char *input) {
    auto ast = parse(input);
    if (!program.segments.empty() &&
        !program.segments.front().instructions.empty() &&
        program.segments.front().instructions.back().type == Instruction::Exit) {
        program.segments.front().instructions.pop_back();
    }
    for (auto &node: ast) {
        node->compile(program, program.segments[0]);
    }
    program.segments.front().instructions.push_back(
            Instruction{
                    .type = Instruction::InstructionType::Exit,
            });
}
Program compile(const char *input) {
    Program program;
    compile(program, input);
    return program;
}
