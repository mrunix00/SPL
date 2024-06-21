#include "vm.h"
#include <cstring>
#include <iostream>
#include <stdexcept>

Program::Program() {
    segments.emplace_back();
}
void Program::addGlobal(const std::string &name, Variable::Type type) {
    Variable variable;
    variable.name = name;
    variable.type = type;
    variable.index = globals.size();
    globals[name] = variable;
}

VM::VM() {
    stackCapacity = 1024;
    stack = malloc(stackCapacity);
}
void VM::newStackFrame(const Segment &segment) {
    StackFrame frame;
    frame.locals.resize(segment.locals.size());
    for (auto &[name, variable]: segment.locals) {
        frame.locals[variable.index] = nullptr;
    }
    callStack.push(frame);
}
void VM::popStackFrame() {
    callStack.pop();
}
void *VM::getLocal(const size_t index) {
    return callStack.top().locals.at(index);
}
void VM::setLocal(const size_t index, void **value) {
    callStack.top().locals[index] = *value;
}
void *VM::getGlobal(size_t index) {
    return globals[index];
}
void VM::setGlobal(const size_t index, void **value) {
    globals[index] = *value;
}
void VM::pushStack(void *value, size_t size) {
    if (stackSize + size > stackCapacity) {
        stackCapacity *= 2;
        stack = realloc(stack, stackCapacity);
    }
    memcpy(static_cast<char *>(stack) + stackSize, value, size);
    stackSize += size;
}
void *VM::popStack(size_t size) {
    void *value = malloc(size);
    stackSize -= size;
    memcpy(value, static_cast<char *>(stack) + stackSize, size);
    return value;
}
void *VM::topStack(size_t size) {
    return static_cast<char *>(stack) + stackSize - size;
}

void VM::run(const Program &program) {
    size_t segmentIndex = 0, instructionIndex = 0;
    globals.reserve(program.globals.size());
    for (;;) {
        auto &segment = program.segments[segmentIndex];
        if (instructionIndex == segment.instructions.size() && segmentIndex == 0) {
            break;
        }
        auto &instruction = segment.instructions[instructionIndex];
        switch (instruction.type) {
            case Instruction::InstructionType::AddI32: {
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a + b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::SubI32: {
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a - b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::MulI32: {
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a * b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::DivI32: {
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a / b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::ModI32: {
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a % b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::LoadI32: {
                pushStack(((void *) &instruction.params.i32), sizeof(int32_t));
            } break;
            case Instruction::InstructionType::StoreGlobalI32: {
                auto val = static_cast<int32_t *>(popStack(sizeof(int32_t)));
                setGlobal(instruction.params.index, (void **) &val);
            } break;
            default:
                throw std::runtime_error("[VM::run] This should not be accessed!");
        }
        instructionIndex++;
    }
}
