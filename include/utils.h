#pragma once

#include "vm.h"

#define GENERATE_EMIT_FUNCTION(OPERATION)                                                                   \
    static inline void emit##OPERATION(Program &program, Segment &segment, const std::string &identifier) { \
        size_t id;                                                                                          \
        bool isLocal;                                                                                       \
        Instruction instruction;                                                                            \
        Variable::Type type;                                                                                \
        if (segment.find_local(identifier) != -1) {                                                         \
            isLocal = true;                                                                                 \
            id = segment.find_local(identifier);                                                            \
            type = segment.locals[identifier].type;                                                         \
        } else if (program.find_global(identifier) != -1) {                                                 \
            isLocal = false;                                                                                \
            id = program.find_global(identifier);                                                           \
            type = program.segments[0].locals[identifier].type;                                             \
        } else {                                                                                            \
            throw std::runtime_error("[Node::compile] Identifier not found: " + identifier);                \
        }                                                                                                   \
        switch (type) {                                                                                     \
            case Variable::Type::I32:                                                                       \
                instruction.type = isLocal ? Instruction::InstructionType::OPERATION##LocalI32              \
                                           : Instruction::InstructionType::OPERATION##GlobalI32;            \
                instruction.params.index = id;                                                              \
                break;                                                                                      \
            case Variable::Type::I64:                                                                       \
                instruction.type = isLocal ? Instruction::InstructionType::OPERATION##LocalI64              \
                                           : Instruction::InstructionType::OPERATION##GlobalI64;            \
                instruction.params.index = id;                                                              \
                break;                                                                                      \
            default:                                                                                        \
                throw std::runtime_error("[Node::compile] Invalid variable type!");                         \
        }                                                                                                   \
        segment.instructions.push_back(instruction);                                                        \
    }
GENERATE_EMIT_FUNCTION(Load)
GENERATE_EMIT_FUNCTION(Store)

static inline Variable::Type varTypeConvert(AbstractSyntaxTree *ast) {
    if (ast->nodeType != AbstractSyntaxTree::Type::Node)
        throw std::runtime_error("[Declaration::compile] Invalid type: " + ast->typeStr);
    auto token = dynamic_cast<Node *>(ast)->token;
    switch (token.type) {
        case I32:
            return Variable::Type::I32;
        case I64:
            return Variable::Type::I64;
        default:
            throw std::runtime_error("[Declaration::compile] Invalid type: " + token.value);
    }
}

#define DECLARE_NUMBER_VAR(TYPE, PARAM)                                                          \
    case TYPE: {                                                                                 \
        segment.instructions.push_back({.type = Instruction::InstructionType::Load##TYPE,        \
                                        .params = {.PARAM = std::stoi(initNode->token.value)}}); \
        segment.instructions.push_back({                                                         \
                .type = segment.id == 0                                                          \
                                ? Instruction::InstructionType::StoreGlobal##TYPE                \
                                : Instruction::InstructionType::StoreLocal##TYPE,                \
                .params = {.index = segment.locals.size()},                                      \
        });                                                                                      \
        segment.declare_variable(identifier.token.value, Variable::Type::TYPE);                  \
    } break;

#define DECLARE_OTHER_VAR(TYPE)                                                   \
    case TYPE: {                                                                  \
        initNode->compile(program, segment);                                      \
        segment.instructions.push_back({                                          \
                .type = segment.id == 0                                           \
                                ? Instruction::InstructionType::StoreGlobal##TYPE \
                                : Instruction::InstructionType::StoreLocal##TYPE, \
                .params = {.index = segment.locals.size()},                       \
        });                                                                       \
        segment.declare_variable(identifier.token.value, Variable::Type::TYPE);   \
    } break;

#define VAR_CASE(OP, TYPE)                                                                           \
    case Variable::Type::TYPE:                                                                       \
        segment.instructions.push_back(Instruction{.type = Instruction::InstructionType::OP##TYPE}); \
        break;

static Variable::Type deduceType(Program &program, Segment &segment, AbstractSyntaxTree *ast) {
    switch (ast->nodeType) {
        case AbstractSyntaxTree::Type::Node: {
            auto token = dynamic_cast<Node *>(ast)->token;
            switch (token.type) {
                case Number: {
                    try {
                        std::stoi(token.value);
                        return Variable::Type::I32;
                    } catch (std::out_of_range &) {
                        goto long64;
                    }
                long64:
                    try {
                        std::stol(token.value);
                        return Variable::Type::I64;
                    } catch (std::exception &) {
                        throw std::runtime_error("Invalid number: " + token.value);
                    }
                }
                case Identifier: {
                    if (segment.find_local(token.value) != -1)
                        return segment.locals[token.value].type;
                    if (program.find_global(token.value) != -1)
                        return program.segments[0].locals[token.value].type;
                    throw std::runtime_error("Identifier not found: " + token.value);
                }
                default:
                    throw std::runtime_error("Invalid type: " + token.value);
            }
        }
        case AbstractSyntaxTree::Type::UnaryExpression: {
            auto unary = dynamic_cast<UnaryExpression *>(ast);
            return deduceType(program, segment, unary->expression);
        }
        case AbstractSyntaxTree::Type::BinaryExpression: {
            auto binary = dynamic_cast<BinaryExpression *>(ast);
            auto left = deduceType(program, segment, binary->left);
            auto right = deduceType(program, segment, binary->right);
            if ((left == Variable::Type::I32 || left == Variable::Type::I64) &&
                (right == Variable::Type::I32 || right == Variable::Type::I64))
                return left == Variable::Type::I64 || right == Variable::Type::I64 ? Variable::Type::I64
                                                                                   : Variable::Type::I32;
            if (left != right)
                throw std::runtime_error("Type mismatch");
            return left;
        }
        case AbstractSyntaxTree::Type::FunctionCall: {
            // TODO: add support for multiple return types
            auto call = dynamic_cast<FunctionCall *>(ast);
            auto function = program.find_function(program.segments[segment.id], call->identifier.token.value);
            if (function == -1)
                throw std::runtime_error("Function not found: " + call->identifier.token.value);
            return Variable::Type::I32;
        }
        default:
            throw std::runtime_error("Invalid type: " + ast->typeStr);
    }
}

enum class GenericInstruction {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    Equal,
    Less,
    Greater,
    GreaterEqual,
    LessEqual,
    NotEqual
};

#define TYPE_CASE(INS)                                                             \
    case GenericInstruction::INS: {                                                \
        switch (type) {                                                            \
            case Variable::Type::I32:                                              \
                return {Instruction::InstructionType::INS##I32};                   \
            case Variable::Type::I64:                                              \
                return {Instruction::InstructionType::INS##I64};                   \
            default:                                                               \
                throw std::runtime_error("[getInstructionWithType] Invalid type"); \
        }                                                                          \
    }
static inline Instruction getInstructionWithType(GenericInstruction instruction, Variable::Type type) {
    switch (instruction) {
        TYPE_CASE(Add)
        TYPE_CASE(Sub)
        TYPE_CASE(Mul)
        TYPE_CASE(Div)
        TYPE_CASE(Mod)
        TYPE_CASE(Equal)
        TYPE_CASE(Less)
        TYPE_CASE(Greater)
        TYPE_CASE(GreaterEqual)
        TYPE_CASE(LessEqual)
        TYPE_CASE(NotEqual)
    }
}
