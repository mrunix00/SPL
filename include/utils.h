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