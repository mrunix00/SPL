#include "utils.h"

void assert(bool condition, const char *message) {
    if (!condition)
        throw std::runtime_error("Assertion failed: " + std::string(message));
}

VariableType *varTypeConvert(AbstractSyntaxTree *ast) {
    if (ast->nodeType == AbstractSyntaxTree::Type::Node) {
        auto token = dynamic_cast<Node *>(ast)->token;
        switch (token.type) {
            case Bool:
                return new VariableType(VariableType::Bool);
            case Int:
                return new VariableType(VariableType::I64);
            case Str:
                return new VariableType(VariableType::Object);
            case Void:
                return new VariableType(VariableType::Void);
            default:
                throw std::runtime_error("[Declaration::compile] Invalid type: " + token.value);
        }
    } else if (ast->nodeType == AbstractSyntaxTree::Type::ArrayType) {
        auto arrayType = dynamic_cast<ArrayType *>(ast);
        return new ArrayObjectType(varTypeConvert(arrayType->type));
    } else {
        throw std::runtime_error("[Declaration::compile] Invalid type: " + ast->typeStr);
    }
}

VariableType::Type biggestType(VariableType::Type first, VariableType::Type second) {
    switch (first) {
        case VariableType::I64:
            switch (second) {
                case VariableType::I64:
                    return first;
                default:
                    throw std::runtime_error("Type mismatch!");
            }
        default:
            throw std::runtime_error("Type mismatch!");
    }
}

VariableType *deduceType(Program &program, Segment &segment, AbstractSyntaxTree *ast) {
    switch (ast->nodeType) {
        case AbstractSyntaxTree::Type::Node: {
            auto token = dynamic_cast<Node *>(ast)->token;
            switch (token.type) {
                case String:
                    return new VariableType(VariableType::Object);
                case True:
                case False:
                    return new VariableType(VariableType::Bool);
                case Number: {
                    try {
                        std::stol(token.value);
                        return new VariableType(VariableType::I64);
                    } catch (std::exception &) {
                        throw std::runtime_error("Invalid number: " + token.value);
                    }
                }
                case Identifier: {
                    if (segment.find_local(token.value) != -1)
                        return new VariableType(segment.locals[token.value].type->type);
                    if (program.find_global(token.value) != -1)
                        return new VariableType(program.segments[0].locals[token.value].type->type);
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
            auto binaryExpr = dynamic_cast<BinaryExpression *>(ast);
            if (binaryExpr->op.type == Less || binaryExpr->op.type == Greater || binaryExpr->op.type == LessEqual ||
                binaryExpr->op.type == GreaterEqual || binaryExpr->op.type == Equal || binaryExpr->op.type == NotEqual)
                return new VariableType(VariableType::Bool);
            auto left = deduceType(program, segment, binaryExpr->left);
            auto right = deduceType(program, segment, binaryExpr->right);
            return new VariableType(biggestType(left->type, right->type));
        }
        case AbstractSyntaxTree::Type::TernaryExpression: {
            auto ternary = dynamic_cast<TernaryExpression *>(ast);
            auto type = deduceType(program, segment, ternary->thenCase);
            return type;
        }
        case AbstractSyntaxTree::Type::FunctionCall: {
            auto call = dynamic_cast<FunctionCall *>(ast);
            if (call->identifier.token.value == "native") {
                return new VariableType(VariableType::NativeLib);
            }
            auto function = program.find_function(segment, call->identifier.token.value);
            return new VariableType(((FunctionType *) function.type)->returnType->type);
        }
        case AbstractSyntaxTree::Type::Declaration: {
            auto declaration = dynamic_cast<Declaration *>(ast);
            if (declaration->type.has_value())
                return varTypeConvert(declaration->type.value());
            return deduceType(program, segment, declaration->value.value());
        }
        case AbstractSyntaxTree::Type::ArrayAccess: {
            auto arrayAccess = dynamic_cast<ArrayAccess *>(ast);
            bool isLocal;
            if (segment.find_local(arrayAccess->identifier.token.value) != -1)
                isLocal = true;
            else if (program.find_global(arrayAccess->identifier.token.value) != -1)
                isLocal = false;
            else
                throw std::runtime_error("Identifier not found: " + arrayAccess->identifier.token.value);
            auto type = (ArrayObjectType *) (isLocal ? segment.locals[arrayAccess->identifier.token.value].type
                                                     : program.segments[0].locals[arrayAccess->identifier.token.value].type);
            return type->elementType;
        } break;
        default:
            throw std::runtime_error("Invalid type: " + ast->typeStr);
    }
}

#define TYPE_CASE(INS)                                                             \
    case GenericInstruction::INS: {                                                \
        switch (type) {                                                            \
            case VariableType::I64:                                                \
                return {Instruction::InstructionType::INS##I64};                   \
            default:                                                               \
                throw std::runtime_error("[getInstructionWithType] Invalid type"); \
        }                                                                          \
    }
Instruction getInstructionWithType(GenericInstruction instruction, VariableType::Type type) {
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

Instruction emitLoad(VariableType::Type type, const Token &token) {
    switch (type) {
        case VariableType::Bool:
        case VariableType::I64:
            return Instruction{
                    .type = Instruction::InstructionType::LoadI64,
                    .params = {.i64 = std::stol(token.value)},
            };
        default:
            throw std::runtime_error("Invalid type: " + token.value);
    }
}

void typeCast(std::vector<Instruction> &instructions, VariableType::Type from, VariableType::Type to) {
    if (from == to)
        return;
    switch (from) {
        default:
            throw std::runtime_error("Invalid type cast");
    }
}

VariableType::Type getInstructionType(const Program &program, const Instruction &instruction) {
    switch (instruction.type) {
        case Instruction::InstructionType::EqualI64:
        case Instruction::InstructionType::LessI64:
        case Instruction::InstructionType::GreaterI64:
        case Instruction::InstructionType::GreaterEqualI64:
        case Instruction::InstructionType::LessEqualI64:
        case Instruction::InstructionType::NotEqualI64:
            return VariableType::Bool;
        case Instruction::InstructionType::LoadI64:
        case Instruction::InstructionType::AddI64:
        case Instruction::InstructionType::SubI64:
        case Instruction::InstructionType::MulI64:
        case Instruction::InstructionType::DivI64:
        case Instruction::InstructionType::ModI64:
        case Instruction::InstructionType::IncrementI64:
        case Instruction::InstructionType::DecrementI64:
        case Instruction::InstructionType::StoreLocalI64:
        case Instruction::InstructionType::LoadLocalI64:
        case Instruction::InstructionType::StoreGlobalI64:
        case Instruction::InstructionType::LoadGlobalI64:
            return VariableType::I64;
        case Instruction::InstructionType::LoadObject:
        case Instruction::InstructionType::StoreLocalObject:
        case Instruction::InstructionType::LoadLocalObject:
        case Instruction::InstructionType::StoreGlobalObject:
        case Instruction::InstructionType::LoadGlobalObject:
        case Instruction::InstructionType::MakeArray:
            return VariableType::Object;
        case Instruction::InstructionType::LoadFromLocalArray:
        case Instruction::InstructionType::LoadFromGlobalArray: {
            auto array_index = instruction.params.index;
            auto locals = program.segments[0].locals;
            for (auto &local: locals) {
                if (local.second.index == array_index) {
                    auto type = (ArrayObjectType *) local.second.type;
                    return type->elementType->type;
                }
            }
        } break;
        case Instruction::InstructionType::Call: {
            auto func_index = instruction.params.index;
            auto function = program.segments[func_index];
            return function.returnType->type;
        }
        case Instruction::InstructionType::Jump:
        case Instruction::InstructionType::JumpIfFalse:
        case Instruction::InstructionType::Return:
        case Instruction::InstructionType::Invalid:
        case Instruction::InstructionType::Exit:
            return VariableType::Invalid;
    }
}
