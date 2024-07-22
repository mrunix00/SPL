#include "vm.h"
#include "utils.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>

Program::Program() {
    segments.emplace_back();
}
size_t Program::find_global(const std::string &identifier) {
    auto &segment = segments.front();
    auto it = segment.locals.find(identifier);
    if (it == segment.locals.end())
        return -1;
    return it->second.index;
}
Variable Program::find_function(const Segment &segment, const std::string &identifier) {
    auto it = segment.functions.find(identifier);
    if (it == segment.functions.end()) {
        it = segments.front().functions.find(identifier);
        if (it == segments.front().functions.end())
            throw std::runtime_error("[Program::find_function] Function not found: " + identifier);
    }
    return it->second;
}

void Segment::declare_variable(const std::string &name, VariableType *varType) {
    locals[name] = Variable(name, varType, locals_capacity, 0);
    if (varType->type != VariableType::Type::Function) {
        locals[name].size = sizeOfType(varType->type);
        locals_capacity += sizeOfType(varType->type);
    }
}
size_t Segment::find_local(const std::string &identifier) {
    auto it = locals.find(identifier);
    if (it == locals.end())
        return -1;
    return it->second.index;
}
void Segment::declare_function(const std::string &name, VariableType *funcType, size_t index) {
    auto function = Variable(name, funcType, index, 0);
    functions[name] = function;
    locals[name] = function;
}

VM::VM() {
    stackCapacity = 1024;
    stack = (uint64_t *) malloc(stackCapacity * sizeof(uint64_t));
    if (stack == nullptr)
        throw std::runtime_error("Memory allocation failure!");
    callStack.push_back(StackFrame{});
}
inline void VM::newStackFrame(const Segment &segment, size_t id) {
    StackFrame frame;
    frame.segmentIndex = id;
    frame.localsSize = segment.locals_capacity;
    frame.locals = malloc(frame.localsSize * sizeof(uint64_t));
    if (frame.locals == nullptr) {
        throw std::runtime_error("Memory allocation failure!");
    }
    callStack.push_back(frame);
    for (size_t i = frame.localsSize - 1; i != -1; i--) {
        if (((uint64_t *) stack)[stackSize - 1] == VariableType::Type::I64) {
            auto val = popDoubleStack();
            setDoubleLocal(--i, val);
        } else {
            auto val = popStack();
            setLocal(i, val);
        }
    }
}
inline void VM::popStackFrame() {
    free(callStack.back().locals);
    callStack.pop_back();
}
inline StackObject VM::getLocal(const size_t index) {
    return ((StackObject *) callStack.back().locals)[index];
}
inline void VM::setLocal(const size_t index, StackObject value) {
    ((StackObject *) callStack.back().locals)[index] = value;
}
inline StackObject VM::getGlobal(size_t index) {
    return ((StackObject *) callStack[0].locals)[index];
}
inline void VM::setGlobal(const size_t index, StackObject value) {
    ((StackObject *) callStack.front().locals)[index] = value;
}
inline DoubleStackObject VM::getDoubleLocal(const size_t index) {
    auto value = ((uint64_t *) callStack.back().locals)[index];
    auto type = ((uint64_t *) callStack.back().locals)[index + 1];
    return {(VariableType::Type) type, value};
}
inline void VM::setDoubleLocal(const size_t index, DoubleStackObject value) {
    ((uint64_t *) callStack.back().locals)[index] = value.value;
    ((uint64_t *) callStack.back().locals)[index + 1] = value.type;
}
inline DoubleStackObject VM::getDoubleGlobal(size_t index) {
    auto value = ((uint64_t *) callStack.front().locals)[index];
    auto type = ((uint64_t *) callStack.front().locals)[index + 1];
    return {(VariableType::Type) type, value};
}
inline void VM::setDoubleGlobal(const size_t index, DoubleStackObject value) {
    ((uint64_t *) callStack.front().locals)[index] = value.value;
    ((uint64_t *) callStack.front().locals)[index + 1] = value.type;
}
inline void VM::pushStack(StackObject value) {
    if (stackSize + 1 > stackCapacity) {
        stackCapacity *= 2;
        auto newStack = realloc(stack, stackCapacity * sizeof(uint64_t));
        if (newStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        stack = newStack;
    }
    ((StackObject *) stack)[stackSize++] = value;
}
inline void VM::pushDoubleStack(DoubleStackObject value) {
    if (stackSize + 2 > stackCapacity) {
        stackCapacity *= 2;
        auto newStack = realloc(stack, stackCapacity * sizeof(uint64_t));
        if (newStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        stack = newStack;
    }
    ((uint64_t *) stack)[stackSize++] = value.value;
    ((uint64_t *) stack)[stackSize++] = value.type;
}
inline StackObject VM::popStack() {
    return ((StackObject *) stack)[--stackSize];
}
inline DoubleStackObject VM::popDoubleStack() {
    auto type = ((uint64_t *) stack)[--stackSize];
    auto value = ((uint64_t *) stack)[--stackSize];
    return {(VariableType::Type) type, value};
}
StackObject VM::topStack() {
    return ((StackObject *) stack)[stackSize - 1];
}
[[maybe_unused]] DoubleStackObject VM::topDoubleStack() {
    auto type = ((uint64_t *) stack)[stackSize - 1];
    auto value = ((uint64_t *) stack)[stackSize - 2];
    return {(VariableType::Type) type, value};
}

void VM::run(const Program &program) {
    if (callStack.front().localsSize != program.segments.front().locals_capacity) {
        callStack.front().localsSize = program.segments.front().locals_capacity;
        auto *newPtr = (uint64_t *) realloc(callStack.front().locals, callStack.front().localsSize * sizeof(uint64_t));
        if (newPtr == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        } else {
            callStack.front().locals = newPtr;
        }
    }
    for (;;) {
        auto &segment = program.segments[callStack.back().segmentIndex];
        if (callStack.back().currentInstruction == segment.instructions.size() && callStack.back().segmentIndex == 0) {
            break;
        }
        auto &instruction = segment.instructions[callStack.back().currentInstruction];
        switch (instruction.type) {
            case Instruction::InstructionType::Invalid:
                throw std::runtime_error("[VM::run] Invalid instruction!");
            case Instruction::InstructionType::AddI32: {
                auto a = popStack();
                auto b = popStack();
                pushStack({VariableType::I32, a.value + b.value});
            } break;
            case Instruction::InstructionType::SubI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::I32, a.value - b.value});
            } break;
            case Instruction::InstructionType::MulI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::I32, a.value * b.value});
            } break;
            case Instruction::InstructionType::DivI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::I32, a.value / b.value});
            } break;
            case Instruction::InstructionType::ModI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::I32, a.value % b.value});
            } break;
            case Instruction::InstructionType::GreaterI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value > b.value});
            } break;
            case Instruction::InstructionType::LessI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value < b.value});
            } break;
            case Instruction::IncrementU32:
            case Instruction::InstructionType::IncrementI32: {
                auto val = popStack();
                pushStack({VariableType::I32, val.value + 1});
            } break;
            case Instruction::DecrementU32:
            case Instruction::InstructionType::DecrementI32: {
                auto val = popStack();
                pushStack({VariableType::I32, val.value - 1});
            } break;
            case Instruction::InstructionType::LoadU32: {
                pushStack({VariableType::U32, static_cast<uint32_t>(instruction.params.u32)});
            } break;
            case Instruction::InstructionType::LoadI32: {
                pushStack({VariableType::Type::I32, static_cast<uint32_t>(instruction.params.i32)});
            } break;
            case Instruction::InstructionType::StoreGlobalI32:
            case Instruction::InstructionType::StoreGlobalU32: {
                auto val = popStack();
                setGlobal(instruction.params.index, val);
            } break;
            case Instruction::InstructionType::LoadGlobalU32:
            case Instruction::InstructionType::LoadGlobalI32: {
                auto val = getGlobal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::InstructionType::StoreLocalU32:
            case Instruction::InstructionType::StoreLocalI32: {
                auto val = popStack();
                setLocal(instruction.params.index, val);
            } break;
            case Instruction::InstructionType::LoadLocalU32:
            case Instruction::InstructionType::LoadLocalI32: {
                auto val = getLocal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::InstructionType::Return:
                popStackFrame();
                continue;
            case Instruction::InstructionType::Call:
                callStack.back().currentInstruction++;
                newStackFrame(program.segments[instruction.params.index], instruction.params.index);
                continue;
            case Instruction::InstructionType::JumpIfFalse: {
                auto cond = popStack();
                if (cond.value == 0) {
                    callStack.back().currentInstruction = instruction.params.index;
                    continue;
                }
            } break;
            case Instruction::InstructionType::Jump:
                callStack.back().currentInstruction = instruction.params.index;
                continue;
            case Instruction::AddI64: {
                auto a = popDoubleStack();
                auto b = popDoubleStack();
                pushDoubleStack({VariableType::I64, a.value + b.value});
            } break;
            case Instruction::SubI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack({VariableType::I64, a.value - b.value});
            } break;
            case Instruction::MulI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack({VariableType::I64, a.value * b.value});
            } break;
            case Instruction::DivI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack({VariableType::I64, a.value / b.value});
            } break;
            case Instruction::ModI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack({VariableType::I64, a.value % b.value});
            } break;
            case Instruction::GreaterI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value > b.value});
            } break;
            case Instruction::LessI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value < b.value});
            } break;
            case Instruction::GreaterEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value >= b.value});
            } break;
            case Instruction::GreaterEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value >= b.value});
            } break;
            case Instruction::LessEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value <= b.value});
            } break;
            case Instruction::LessEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value <= b.value});
            } break;
            case Instruction::EqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value == b.value});
            } break;
            case Instruction::EqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value == b.value});
            } break;
            case Instruction::NotEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack({VariableType::Type::Bool, a.value != b.value});
            } break;
            case Instruction::NotEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack({VariableType::Type::Bool, a.value != b.value});
            } break;
            case Instruction::IncrementI64: {
                auto val = popDoubleStack();
                pushDoubleStack({VariableType::I64, val.value + 1});
            } break;
            case Instruction::DecrementI64: {
                auto val = popDoubleStack();
                pushDoubleStack({VariableType::I64, val.value - 1});
            } break;
            case Instruction::StoreGlobalObject:
            case Instruction::StoreGlobalI64: {
                auto val = popDoubleStack();
                setDoubleGlobal(instruction.params.index, val);
            } break;
            case Instruction::StoreLocalObject:
            case Instruction::StoreLocalI64: {
                auto val = popDoubleStack();
                setDoubleLocal(instruction.params.index, val);
            } break;
            case Instruction::LoadI64: {
                pushDoubleStack({VariableType::I64, static_cast<uint64_t>(instruction.params.i64)});
            } break;
            case Instruction::LoadGlobalObject:
            case Instruction::LoadGlobalI64: {
                auto val = getDoubleGlobal(instruction.params.index);
                pushDoubleStack(val);
            } break;
            case Instruction::LoadLocalObject:
            case Instruction::LoadLocalI64: {
                auto val = getDoubleLocal(instruction.params.index);
                pushDoubleStack(val);
            } break;
            case Instruction::ConvertI32toI64:
            case Instruction::ConvertU32toI64: {
                auto val = popStack();
                pushDoubleStack({VariableType::I64, val.value});
            } break;
            case Instruction::LoadObject: {
                pushDoubleStack({VariableType::Object, reinterpret_cast<uint64_t>(instruction.params.ptr)});
            } break;
        }
        callStack.back().currentInstruction++;
    }
}
