#pragma once

#include "ast.h"
#include "vm.h"
#include <stdexcept>

#define GENERATE_EMIT_FUNCTION(OPERATION)                                                                   \
    static inline void emit##OPERATION(Program &program, Segment &segment, const std::string &identifier) { \
        size_t id;                                                                                          \
        bool isLocal;                                                                                       \
        Instruction instruction;                                                                            \
        VariableType *type;                                                                                 \
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
        switch (type->type) {                                                                               \
            case VariableType::Type::Object:                                                                \
                instruction.type = isLocal ? Instruction::InstructionType::OPERATION##LocalObject           \
                                           : Instruction::InstructionType::OPERATION##GlobalObject;         \
                instruction.params.index = id;                                                              \
                break;                                                                                      \
            case VariableType::Type::I64:                                                                   \
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

template<typename T>
inline T convert(const std::string &value) {
    return static_cast<T>(std::stoll(value));
}
template<>
inline uint32_t convert<uint32_t>(const std::string &value) {
    return static_cast<uint32_t>(std::stoull(value));
}

#define VAR_CASE(OP, TYPE)                                                                           \
    case VariableType::Type::TYPE:                                                                   \
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
VariableType *varTypeConvert(AbstractSyntaxTree *ast);
VariableType *deduceType(Program &program, Segment &segment, AbstractSyntaxTree *ast);
Instruction getInstructionWithType(GenericInstruction instruction, VariableType::Type type);
Instruction emitLoad(VariableType::Type, const Token &token);
void typeCast(std::vector<Instruction> &instructions, VariableType::Type from, VariableType::Type to);
VariableType::Type biggestType(VariableType::Type first, VariableType::Type second);
VariableType::Type getInstructionType(const Program &program, const Instruction &instruction);

#ifndef __cpp_lib_bit_cast
namespace std {
    template<typename To, typename From>
    To bit_cast(const From &from) {
        static_assert(sizeof(To) == sizeof(From));
        To to;
        std::memcpy(&to, &from, sizeof(To));
        return to;
    }
}// namespace std
#endif
