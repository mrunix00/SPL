#include "vm.h"
#include "utils.h"
#include <cstring>
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
    stack = (uint32_t *) malloc(stackCapacity);
    if (stack == nullptr)
        throw std::runtime_error("Memory allocation failure!");
    callStack.push_back(StackFrame{});
}
inline void VM::newStackFrame(const Segment &segment, size_t id) {
    StackFrame frame;
    frame.segmentIndex = id;
    frame.localsSize = segment.locals_capacity;
    frame.locals = (uint32_t *) malloc(frame.localsSize);
    if (frame.locals == nullptr) {
        throw std::runtime_error("Memory allocation failure!");
    }
    callStack.push_back(frame);
    for (size_t i = segment.locals_capacity - 1; i != -1; i--) {
        auto val = popStack();
        setLocal(i, val);
    }
}
inline void VM::popStackFrame() {
    free(callStack.back().locals);
    callStack.pop_back();
}
inline uint32_t VM::getLocal(const size_t index) {
    return callStack.back().locals[index];
}
inline void VM::setLocal(const size_t index, uint32_t value) {
    callStack.back().locals[index] = value;
}
inline uint32_t VM::getGlobal(size_t index) {
    return callStack[0].locals[index];
}
inline void VM::setGlobal(const size_t index, uint32_t value) {
    callStack.front().locals[index] = value;
}
inline uint64_t VM::getDoubleLocal(const size_t index) {
    return (static_cast<uint64_t>(getLocal(index)) << 32) | (static_cast<uint32_t>(getLocal(index + 1)) & 0xFFFFFFFF);
}
inline void VM::setDoubleLocal(const size_t index, uint64_t value) {
    setLocal(index, value >> 32);
    setLocal(index + 1, value & 0xFFFFFFFF);
}
inline uint64_t VM::getDoubleGlobal(size_t index) {
    return (uint64_t) getGlobal(index) << 32 | getGlobal(index + 1);
}
inline void VM::setDoubleGlobal(const size_t index, uint64_t value) {
    callStack.front().locals[index] = value >> 32;
    callStack.front().locals[index + 1] = value & 0xFFFFFFFF;
}
inline void VM::pushStack(uint32_t value) {
    if (stackSize + 1 > stackCapacity) {
        stackCapacity *= 2;
        stack = (uint32_t *) realloc(stack, stackCapacity);
        if (stack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
    }
    stack[stackSize++] = value;
}
inline void VM::pushDoubleStack(uint64_t value) {
    if (stackSize + 2 > stackCapacity) {
        stackCapacity *= 2;
        stack = (uint32_t *) realloc(stack, stackCapacity);
    }
    pushStack(value >> 32);
    pushStack(value & 0xFFFFFFFF);
}
inline uint32_t VM::popStack() {
    return stack[--stackSize];
}
inline uint64_t VM::popDoubleStack() {
    auto low = popStack();
    auto high = popStack();
    return (static_cast<uint64_t>(high) << 32) | (static_cast<uint32_t>(low) & 0xFFFFFFFF);
}
uint32_t VM::topStack() {
    return stack[stackSize - 1];
}
[[maybe_unused]] uint64_t VM::topDoubleStack() {
    auto low = stack[stackSize - 1];
    auto high = stack[stackSize - 2];
    return (static_cast<uint64_t>(high) << 32) | (static_cast<uint32_t>(low) & 0xFFFFFFFF);
}

void VM::run(const Program &program) {
    if (callStack.front().localsSize != program.segments.front().locals_capacity) {
        auto *newPtr = (uint32_t *) realloc(callStack.front().locals, program.segments.front().locals.size());
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
                pushStack(a + b);
            } break;
            case Instruction::InstructionType::SubI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a - b);
            } break;
            case Instruction::InstructionType::MulI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a * b);
            } break;
            case Instruction::InstructionType::DivI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a / b);
            } break;
            case Instruction::InstructionType::ModI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a % b);
            } break;
            case Instruction::InstructionType::GreaterI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a > b);
            } break;
            case Instruction::InstructionType::LessI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a < b);
            } break;
            case Instruction::IncrementU32:
            case Instruction::InstructionType::IncrementI32: {
                auto val = popStack();
                pushStack(val + 1);
            } break;
            case Instruction::DecrementU32:
            case Instruction::InstructionType::DecrementI32: {
                auto val = popStack();
                pushStack(val - 1);
            } break;
            case Instruction::InstructionType::LoadU32:
            case Instruction::InstructionType::LoadI32: {
                pushStack(instruction.params.i32);
            } break;
            case Instruction::InstructionType::StoreGlobalU32:
            case Instruction::InstructionType::StoreGlobalI32: {
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
                if (cond == 0) {
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
                pushDoubleStack(a + b);
            } break;
            case Instruction::SubI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack(a - b);
            } break;
            case Instruction::MulI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack(a * b);
            } break;
            case Instruction::DivI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack(a / b);
            } break;
            case Instruction::ModI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushDoubleStack(a % b);
            } break;
            case Instruction::GreaterI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a > b);
            } break;
            case Instruction::LessI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a < b);
            } break;
            case Instruction::GreaterEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a >= b);
            } break;
            case Instruction::GreaterEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a >= b);
            } break;
            case Instruction::LessEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a <= b);
            } break;
            case Instruction::LessEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a <= b);
            } break;
            case Instruction::EqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a == b);
            } break;
            case Instruction::EqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a == b);
            } break;
            case Instruction::NotEqualI32: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a != b);
            } break;
            case Instruction::NotEqualI64: {
                auto b = popDoubleStack();
                auto a = popDoubleStack();
                pushStack(a != b);
            } break;
            case Instruction::IncrementI64: {
                pushDoubleStack(popDoubleStack() + 1);
            } break;
            case Instruction::DecrementI64: {
                auto val = popDoubleStack();
                pushDoubleStack(val - 1);
            } break;
            case Instruction::StoreGlobalI64: {
                auto val = popDoubleStack();
                setDoubleGlobal(instruction.params.index, val);
            } break;
            case Instruction::StoreLocalI64: {
                auto val = popDoubleStack();
                setDoubleLocal(instruction.params.index, val);
            } break;
            case Instruction::LoadI64: {
                pushDoubleStack(instruction.params.i64);
            } break;
            case Instruction::LoadGlobalI64: {
                auto val = getDoubleGlobal(instruction.params.index);
                pushDoubleStack(val);
            } break;
            case Instruction::LoadLocalI64: {
                auto val = getDoubleLocal(instruction.params.index);
                pushDoubleStack(val);
            } break;
            case Instruction::ConvertI32toI64:
            case Instruction::ConvertU32toI64: {
                auto val = popStack();
                pushDoubleStack(val);
            } break;
        }
        callStack.back().currentInstruction++;
    }
}