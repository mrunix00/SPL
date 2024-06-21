#include "vm.h"
#include <cstring>
#include <stdexcept>

Program::Program() {
    segments.emplace_back();
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
    return callStack.top().locals[index];
}
void VM::setLocal(const size_t index, void **value) {
    callStack.top().locals[index] = *value;
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
    for (;;) {
        auto &segment = program.segments[segmentIndex];
        auto &instruction = segment.instructions[instructionIndex];
        switch (instruction.type) {
            case Instruction::InstructionType::AddI32: {
                int32_t a = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t b = *static_cast<int32_t *>(popStack(sizeof(int32_t)));
                int32_t result = a + b;
                pushStack(&result, sizeof(int32_t));
            } break;
            case Instruction::InstructionType::LoadI32: {
                pushStack(((void *) &instruction.params.i32), sizeof(int32_t));
            } break;
            default:
                throw std::runtime_error("Invalid instruction type: " + std::to_string(instruction.type));
        }
        instructionIndex++;
        if (instructionIndex == segment.instructions.size() && segmentIndex == 0) {
            break;
        }
    }
}
