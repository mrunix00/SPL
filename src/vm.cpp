#include "vm.h"
#include "utils.h"
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
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
    locals[name] = Variable(name, varType, locals_capacity);
    if (varType->type != VariableType::Type::Function) {
        locals_capacity++;
    }
}
size_t Segment::find_local(const std::string &identifier) {
    auto it = locals.find(identifier);
    if (it == locals.end())
        return -1;
    return it->second.index;
}
void Segment::declare_function(const std::string &name, VariableType *funcType, size_t index) {
    auto function = Variable(name, funcType, index);
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
VM::~VM() {
    free(stack);
    for (auto stackFrame: callStack)
        free(stackFrame.locals);
}
inline void VM::newStackFrame(const Segment &segment) {
    auto locals = (uint64_t *) malloc(segment.locals_capacity * sizeof(uint64_t));
    if (locals == nullptr) {
        throw std::runtime_error("Memory allocation failure!");
    }
    callStack.push_back({
            .locals = locals,
            .localsSize = segment.locals_capacity,
            .segmentIndex = segment.id,
            .currentInstruction = 0,
    });
    for (size_t i = segment.number_of_args - 1; i != -1; i--) {
        setLocal(i, popStack());
    }
}
inline void VM::popStackFrame() {
    free(callStack.back().locals);
    callStack.pop_back();
}
inline uint64_t VM::getLocal(const size_t index) const {
    return callStack.back().locals[index];
}
inline void VM::setLocal(const size_t index, uint64_t value) {
    callStack.back().locals[index] = value;
}
inline uint64_t VM::getGlobal(size_t index) const {
    return callStack[0].locals[index];
}
inline void VM::setGlobal(const size_t index, uint64_t value) {
    callStack[0].locals[index] = value;
}
inline void VM::pushStack(uint64_t value) {
    if (stackSize + 1 > stackCapacity) {
        stackCapacity *= 2;
        auto newStack = (uint64_t *) realloc(stack, stackCapacity * sizeof(uint64_t));
        if (newStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        stack = newStack;
    }
    stack[stackSize++] = value;
}
inline uint64_t VM::popStack() {
    return stack[--stackSize];
}
uint64_t VM::topStack() const {
    return stack[stackSize - 1];
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
    auto *segment = &program.segments[callStack.back().segmentIndex];
    for (;;) {
        auto &instruction = segment->instructions[callStack.back().currentInstruction];
        switch (instruction.type) {
            case Instruction::InstructionType::Invalid:
                throw std::runtime_error("[VM::run] Invalid instruction!");
            case Instruction::InstructionType::Return: {
                popStackFrame();
                segment = &program.segments[callStack.back().segmentIndex];
                continue;
            }
            case Instruction::InstructionType::Call: {
                callStack.back().currentInstruction++;
                newStackFrame(program.segments[instruction.params.index]);
                segment = &program.segments[instruction.params.index];
                continue;
            }
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
                auto a = popStack();
                auto b = popStack();
                pushStack(a + b);
            } break;
            case Instruction::SubI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a - b);
            } break;
            case Instruction::MulI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a * b);
            } break;
            case Instruction::DivI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a / b);
            } break;
            case Instruction::ModI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a % b);
            } break;
            case Instruction::GreaterI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a > b);
            } break;
            case Instruction::LessI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a < b);
            } break;
            case Instruction::GreaterEqualI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a >= b);
            } break;
            case Instruction::LessEqualI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a <= b);
            } break;
            case Instruction::EqualI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a == b);
            } break;
            case Instruction::NotEqualI64: {
                auto b = popStack();
                auto a = popStack();
                pushStack(a != b);
            } break;
            case Instruction::IncrementI64: {
                auto val = popStack();
                pushStack(val + 1);
            } break;
            case Instruction::DecrementI64: {
                auto val = popStack();
                pushStack(val - 1);
            } break;
            case Instruction::StoreGlobalObject:
            case Instruction::StoreGlobalI64: {
                auto val = popStack();
                setGlobal(instruction.params.index, val);
            } break;
            case Instruction::StoreLocalObject:
            case Instruction::StoreLocalI64: {
                auto val = popStack();
                setLocal(instruction.params.index, val);
            } break;
            case Instruction::LoadI64: {
                pushStack(instruction.params.i64);
            } break;
            case Instruction::LoadGlobalObject:
            case Instruction::LoadGlobalI64: {
                auto val = getGlobal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::LoadLocalObject:
            case Instruction::LoadLocalI64: {
                auto val = getLocal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::LoadObject: {
                pushStack(std::bit_cast<uint64_t>(instruction.params.ptr));
            } break;
            case Instruction::MakeArray: {
                auto data = (uint64_t *) malloc(instruction.params.index * sizeof(uint64_t));
                if (data == nullptr) {
                    throw std::runtime_error("Memory allocation failure!");
                }
                for (size_t i = instruction.params.index - 1; i != -1; i--) {
                    auto val = popStack();
                    data[i] = val;
                }
                auto newObject = new ArrayObject(instruction.params.index, data);
                pushStack(std::bit_cast<uint64_t>(newObject));
            } break;
            case Instruction::LoadFromLocalArray: {
                auto index = popStack();
                auto array = std::bit_cast<ArrayObject *>(getLocal(instruction.params.index));
                if (index >= array->size) {
                    throw std::runtime_error("[VM::run] Array index out of bounds!");
                }
                pushStack(array->data[index]);
            } break;
            case Instruction::LoadFromGlobalArray: {
                auto index = popStack();
                auto array = std::bit_cast<ArrayObject *>(getGlobal(instruction.params.index));
                if (index >= array->size) {
                    throw std::runtime_error("[VM::run] Array index out of bounds!");
                }
                pushStack(array->data[index]);
            } break;
            case Instruction::AppendToArray: {
                auto array = std::bit_cast<ArrayObject *>(popStack());
                auto val = popStack();
                auto data = (uint64_t*) realloc(array->data, (array->size + 1) * sizeof(uint64_t));
                if (data == nullptr) {
                    throw std::runtime_error("Memory allocation failure!");
                }
                array->data = data;
                array->data[array->size++] = val;
                pushStack(std::bit_cast<uint64_t>(array));
            } break;
            case Instruction::Exit:
                return;
        }
        callStack.back().currentInstruction++;
    }
}
