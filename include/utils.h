#pragma once

#include "ast.h"
#include "vm.h"
#include <stdexcept>

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

void assert(bool condition, const char *message);
Variable::Type varTypeConvert(AbstractSyntaxTree *ast);
Variable::Type deduceType(Program &program, Segment &segment, AbstractSyntaxTree *ast);
Instruction getInstructionWithType(GenericInstruction instruction, Variable::Type type);
Instruction emitLoad(Variable::Type, const Token &token);
void typeCast(std::vector<Instruction> &instructions, Variable::Type from, Variable::Type to);