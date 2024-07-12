#include "utils.h"

void assert(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error("Assertion failed: " + std::string(message));
}

Variable::Type varTypeConvert(AbstractSyntaxTree *ast) {
    if (ast->nodeType != AbstractSyntaxTree::Type::Node)
        throw std::runtime_error("[Declaration::compile] Invalid type: " + ast->typeStr);
    auto token = dynamic_cast<Node *>(ast)->token;
    static std::unordered_map<int, Variable::Type> types = {
            {I32, Variable::Type::I32},
            {I64, Variable::Type::I64},
            {U32, Variable::Type::U32},
    };
    if (types.find(token.type) == types.end())
        throw std::runtime_error("[Declaration::compile] Invalid type: " + token.value);
    return types.at(token.type);
}

Variable::Type biggestType(Variable::Type first, Variable::Type second) {
    switch (first) {
        case Variable::Type::I32:
            switch (second) {
                case Variable::Type::I32:
                    return first;
                case Variable::Type::U32:
                case Variable::Type::I64:
                    return second;
                default:
                    throw std::runtime_error("This should not be accessed!");
            }
        case Variable::Type::I64:
            switch (second) {
                case Variable::Type::I32:
                case Variable::Type::U32:
                case Variable::Type::I64:
                    return first;
                default:
                    throw std::runtime_error("Type mismatch!");
            }
        case Variable::Type::U32:
            switch (second) {
                case Variable::Type::I32:
                case Variable::Type::U32:
                    return first;
                case Variable::Type::I64:
                    return second;
                default:
                    throw std::runtime_error("Type mismatch!");
            }
        default:
            throw std::runtime_error("Type mismatch!");
    }
}

Variable::Type deduceType(Program &program, Segment &segment, AbstractSyntaxTree *ast) {
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
            return biggestType(left, right);
        }
        case AbstractSyntaxTree::Type::FunctionCall: {
            // TODO: add support for other return types
            auto call = dynamic_cast<FunctionCall *>(ast);
            auto function = program.find_function(segment, call->identifier.token.value);
            if (function == -1)
                throw std::runtime_error("Function not found: " + call->identifier.token.value);
            return Variable::Type::I32;
        }
        default:
            throw std::runtime_error("Invalid type: " + ast->typeStr);
    }
}

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
Instruction getInstructionWithType(GenericInstruction instruction, Variable::Type type) {
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

Instruction emitLoad(Variable::Type type, const Token &token) {
    switch (type) {
        case Variable::Type::I32:
            return Instruction{
                    .type = Instruction::InstructionType::LoadI32,
                    .params = {.i32 = std::stoi(token.value)},
            };
        case Variable::Type::U32:
            return Instruction{
                    .type = Instruction::InstructionType::LoadU32,
                    .params = {.u32 = (uint32_t) std::stoul(token.value)},
            };
        case Variable::Type::I64:
            return Instruction{
                    .type = Instruction::InstructionType::LoadI64,
                    .params = {.i64 = std::stol(token.value)},
            };
        default:
            throw std::runtime_error("Invalid type: " + token.value);
    }
}

void typeCast(std::vector<Instruction> &instructions, Variable::Type from, Variable::Type to) {
    if (from == to)
        return;
    switch (from) {
        case Variable::Type::I32:
            switch (to) {
                case Variable::Type::U32:
                    return;
                case Variable::Type::I64:
                    return instructions.push_back({.type = Instruction::InstructionType::ConvertI32toI64});
                default:
                    throw std::runtime_error("Invalid type cast");
            }
        case Variable::Type::U32:
            switch (to) {
                case Variable::Type::I32:
                    return;
                case Variable::Type::I64:
                    return instructions.push_back({.type = Instruction::InstructionType::ConvertU32toI64});
                default:
                    throw std::runtime_error("Invalid type cast");
            }
        default:
            throw std::runtime_error("Invalid type cast");
    }
}
