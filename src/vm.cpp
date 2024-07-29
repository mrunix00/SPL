#include "vm.h"
#include "utils.h"

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
}
VM::~VM() {
    free(stack);
}
void VM::newStackFrame(const Segment &segment) {
    auto &old_stack_ptr = stack[stackSize - 3];
    auto stack_frame_size = segment.stack_depth + segment.locals_capacity + 3;
    stackSize += stack_frame_size;
    if (stackSize >= stackCapacity) {
        auto newStack = (uint64_t *) realloc(stack, segment.locals_capacity * sizeof(uint64_t));
        if (newStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        stack = newStack;
    }
    stack[stackSize - 1] = segment.id;                  // new segment id
    stack[stackSize - 2] = 0;                           // current instruction index
    stack[stackSize - 3] = stackSize - stack_frame_size;//stack pointer

    for (size_t i = segment.locals_capacity - 1; i != -1; i--) {
        auto value = stack[--old_stack_ptr];
        setLocal(i, value);
    }
}
inline void VM::popStackFrame() {
    auto stack_ptr = stack[stackSize - 3];
    stackSize = stack_ptr;
}
inline uint64_t VM::getLocal(const size_t index) const {
    return stack[stackSize - 4 - index];
}
inline void VM::setLocal(const size_t index, uint64_t value) {
    stack[stackSize - 4 - index] = value;
}
inline uint64_t VM::getGlobal(size_t index) const {
    return stack[globalsPtr - index];
}
inline void VM::setGlobal(const size_t index, uint64_t value) {
    stack[globalsPtr - index] = value;
}
inline void VM::pushStack(uint64_t value) {
    auto &stack_ptr = stack[stackSize - 3];
    stack[stack_ptr++] = value;
}
uint64_t VM::popStack() {
    auto &stack_ptr = stack[stackSize - 3];
    return stack[--stack_ptr];
}
uint64_t VM::topStack() const {
    auto stack_ptr = stack[stackSize - 3];
    return stack[stack_ptr - 1];
}
inline size_t VM::getCurrentInstruction() const {
    return stack[stackSize - 2];
}
inline size_t VM::getCurrentSegment() const {
    return stack[stackSize - 1];
}

void VM::run(const Program &program) {
    auto mainSegment = program.segments.front();
    auto mainSegmentOperandsStackDepth = getStackDepth(program, mainSegment.instructions);
    auto newStackSize = mainSegmentOperandsStackDepth + mainSegment.locals_capacity + 3;
    if (stackSize == 0) {
        stackSize = newStackSize;
        memset(stack, 0, stackSize * sizeof(uint64_t));
        numberOfGlobals = mainSegment.locals_capacity;
        globalsPtr = mainSegmentOperandsStackDepth + numberOfGlobals - 1;
    } else if (stackSize != newStackSize) {
        auto newStack = (uint64_t *) malloc(stackCapacity * sizeof(uint64_t));
        if (newStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        memset(newStack, 0, newStackSize * sizeof(uint64_t));
        auto newGlobalsPtr = mainSegmentOperandsStackDepth + mainSegment.locals_capacity - 1;
        for (size_t i = 0; i < numberOfGlobals; i++) {
            newStack[newGlobalsPtr - i] = stack[globalsPtr - i];
        }
        free(stack);
        stack = (uint64_t *) newStack;
        stackSize = newStackSize;
        globalsPtr = newGlobalsPtr;
        numberOfGlobals = mainSegment.locals_capacity;
    }

    for (;;) {
        auto &segment = program.segments[getCurrentSegment()];
        if (getCurrentInstruction() == segment.instructions.size() && getCurrentSegment() == 0) {
            break;
        }
        auto &instruction = segment.instructions[getCurrentInstruction()];
        switch (instruction.type) {
            case Instruction::InstructionType::Invalid:
                throw std::runtime_error("[VM::run] Invalid instruction!");
            case Instruction::InstructionType::Return: {
                auto ret = popStack();
                popStackFrame();
                pushStack(ret);
                continue;
            }
            case Instruction::InstructionType::Call: {
                stack[stackSize - 2]++;
                newStackFrame(program.segments[instruction.params.index]);
                continue;
            }
            case Instruction::InstructionType::JumpIfFalse: {
                auto cond = popStack();
                if (cond == 0) {
                    stack[stackSize - 2] = instruction.params.index;
                    continue;
                }
            } break;
            case Instruction::InstructionType::Jump:
                stack[stackSize - 2] = instruction.params.index;
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
        }
        stack[stackSize - 2]++;
    }
}
