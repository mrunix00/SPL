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
            case Variable::Type::U32:                                                                       \
                instruction.type = isLocal ? Instruction::InstructionType::OPERATION##LocalU32              \
                                           : Instruction::InstructionType::OPERATION##GlobalU32;            \
                instruction.params.index = id;                                                              \
                break;                                                                                      \
            default:                                                                                        \
                throw std::runtime_error("[Node::compile] Invalid variable type!");                         \
        }                                                                                                   \
        segment.instructions.push_back(instruction);                                                        \
    }
GENERATE_EMIT_FUNCTION(Load)
GENERATE_EMIT_FUNCTION(Store)

template<typename T>
inline T convert(const std::string &value) {
    return static_cast<T>(std::stoll(value));
}
template<>
inline uint32_t convert<uint32_t>(const std::string &value) {
    return static_cast<uint32_t>(std::stoull(value));
}

#define DECLARE_VAR_CASE(TYPE, PARAM, CONVERT_TYPE)                                                    \
    case TYPE: {                                                                                       \
        if (!value.has_value()) {                                                                      \
            segment.instructions.push_back({                                                           \
                    .type = Instruction::InstructionType::Load##TYPE,                                  \
                    .params = {.PARAM = 0},                                                            \
            });                                                                                        \
        } else if (((Node *) value.value())->token.type == Number) {                                   \
            segment.instructions.push_back({                                                           \
                    .type = Instruction::InstructionType::Load##TYPE,                                  \
                    .params = {.PARAM = convert<CONVERT_TYPE>(((Node *) value.value())->token.value)}, \
            });                                                                                        \
        } else {                                                                                       \
            ((Node *) value.value())->compile(program, segment);                                       \
        }                                                                                              \
        segment.declare_variable(identifier.token.value, Variable::Type::TYPE);                        \
        segment.instructions.push_back({                                                               \
                .type = segment.id == 0                                                                \
                                ? Instruction::InstructionType::StoreGlobal##TYPE                      \
                                : Instruction::InstructionType::StoreLocal##TYPE,                      \
                .params = {.index = segment.find_local(identifier.token.value)},                       \
        });                                                                                            \
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
size_t sizeOfType(Variable::Type type);