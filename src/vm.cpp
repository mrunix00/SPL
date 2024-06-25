#include "vm.h"
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
size_t Program::find_function(const Segment &segment, const std::string &identifier) {
    auto it = segment.functions.find(identifier);
    if (it == segment.functions.end()) {
        it = segments.front().functions.find(identifier);
        if (it == segments.front().functions.end())
            return -1;
    }
    return it->second;
}

void Segment::declare_variable(const std::string &name, Variable::Type type) {
    locals[name] = Variable{name, type, locals.size()};
}
size_t Segment::find_local(const std::string &identifier) {
    auto it = locals.find(identifier);
    if (it == locals.end())
        return -1;
    return it->second.index;
}
void Segment::declare_function(const std::string &name, size_t index) {
    functions[name] = index;
}

VM::VM() {
    stackCapacity = 1024;
    stack = malloc(stackCapacity);
    callStack.push_back(StackFrame{});
}
inline void VM::newStackFrame(const Segment &segment, size_t id) {
    StackFrame frame;
    frame.segmentIndex = id;
    frame.number_of_locals = segment.locals.size();
    frame.locals = (void **) malloc(frame.number_of_locals * sizeof(void *));
    callStack.push_back(frame);
    for (auto &[name, variable]: segment.locals) {
        auto val = popStack(sizeof(int32_t));
        setLocal(variable.index, &val);
    }
}
inline void VM::popStackFrame() {
    for (size_t i = 0; i < callStack.back().number_of_locals; i++)
        free(callStack.back().locals[i]);
    free(callStack.back().locals);
    callStack.pop_back();
}
inline void *VM::getLocal(const size_t index) {
    return callStack.back().locals[index];
}
inline void VM::setLocal(const size_t index, void **value) {
    callStack.back().locals[index] = *value;
}
inline void *VM::getGlobal(size_t index) {
    return callStack[0].locals[index];
}
inline void VM::setGlobal(const size_t index, void **value) {
    callStack[0].locals[index] = *value;
}
inline void VM::pushStack(void *value, size_t size) {
    if (stackSize + size > stackCapacity) {
        stackCapacity *= 2;
        stack = realloc(stack, stackCapacity);
    }
    memcpy(static_cast<char *>(stack) + stackSize, value, size);
    stackSize += size;
}
inline void *VM::popStack(size_t size) {
    void *value = malloc(size);
    stackSize -= size;
    memcpy(value, static_cast<char *>(stack) + stackSize, size);
    return value;
}
void *VM::topStack(size_t size) {
    return static_cast<char *>(stack) + stackSize - size;
}

void VM::run(const Program &program) {
    callStack.front().locals = (void **) malloc(program.segments.front().locals.size() * sizeof(void *));
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
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto result = *a + *b;
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::SubI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto result = *a - *b;
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::MulI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto result = (*a) * (*b);
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::DivI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto result = *a / *b;
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::ModI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto result = *a % *b;
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::GreaterI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = (*a) > (*b);
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::LessI32: {
                auto b = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                auto a = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = (*a) < (*b);
                free(a);
                free(b);
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::LoadI32: {
                pushStack(((void *) &instruction.params.i32), sizeof(int32_t));
            } break;
            case Instruction::InstructionType::StoreGlobalI32: {
                auto val = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                setGlobal(instruction.params.index, (void **) &val);
            } break;
            case Instruction::InstructionType::LoadGlobalI32: {
                auto val = static_cast<int32_t *>(getGlobal(instruction.params.index));
                pushStack((void *) val, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::StoreLocalI32: {
                auto val = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                setLocal(instruction.params.index, (void **) &val);
            } break;
            case Instruction::InstructionType::LoadLocalI32: {
                auto val = static_cast<int32_t *>(getLocal(instruction.params.index));
                pushStack((void *) val, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::Return:
                popStackFrame();
                continue;
            case Instruction::InstructionType::Call:
                callStack.back().currentInstruction++;
                newStackFrame(program.segments[instruction.params.index], instruction.params.index);
                continue;
            case Instruction::InstructionType::JumpIfFalse: {
                auto val = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                if (*val == 0) {
                    callStack.back().currentInstruction = instruction.params.index;
                    free(val);
                    continue;
                }
                free(val);
            } break;
            case Instruction::InstructionType::Jump:
                callStack.back().currentInstruction = instruction.params.index;
                continue;
            default:
                throw std::runtime_error("[VM::run] Unimplemented instruction!");
        }
        callStack.back().currentInstruction++;
    }
}
