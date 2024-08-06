#include "vm.h"
#include "utils.h"
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
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
    switch (varType->type) {
        case VariableType::Object:
        case VariableType::NativeLib:
        case VariableType::Array:
            locals[name] = Variable(name, varType, number_of_local_ptr);
            number_of_local_ptr++;
            break;
        default:
            locals[name] = Variable(name, varType, number_of_locals);
            if (varType->type != VariableType::Type::Function) {
                number_of_locals++;
            }
            break;
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
    pointersStackCapacity = 1024;
    stack = (uint64_t *) malloc(stackCapacity * sizeof(uint64_t));
    pointersStack = (Object **) malloc(pointersStackCapacity * sizeof(Object *));
    if (stack == nullptr)
        throw std::runtime_error("Memory allocation failure!");
    callStack.push_back(StackFrame{});
}
VM::~VM() {
    free(stack);
    if (pointersStack == nullptr)
        return;
    for (auto i = 0; i < pointersStackSize; i++)
        delete pointersStack[i];
    free(pointersStack);
}
inline void VM::newStackFrame(const Segment &segment) {
    uint64_t *locals{};
    Object **pointers{};
    if (segment.number_of_locals != 0) {
        locals = (uint64_t *) malloc(segment.number_of_locals * sizeof(uint64_t));
        if (locals == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
    }
    if (segment.number_of_local_ptr != 0) {
        pointers = (Object **) malloc(segment.number_of_local_ptr * sizeof(Object *));
        memset(pointers, 0, segment.number_of_local_ptr * sizeof(Object *));
        if (pointers == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
    }
    callStack.push_back({
            .locals = locals,
            .localPointers = pointers,
            .localsSize = segment.number_of_locals,
            .localPointersSize = segment.number_of_local_ptr,
            .segmentIndex = segment.id,
            .currentInstruction = 0,
    });
    for (size_t i = segment.number_of_args - 1; i != -1; i--)
        setLocal(i, popStack());
    for (size_t i = segment.number_of_arg_ptr - 1; i != -1; i--)
        setPointer(i, popPointer());
}
inline void VM::popStackFrame() {
    free(callStack.back().locals);
    free(callStack.back().localPointers);
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
Object *VM::getPointer(size_t index) const {
    return callStack.back().localPointers[index];
}
void VM::setPointer(size_t index, Object *object) {
    callStack.back().localPointers[index] = object;
}
Object *VM::getGlobalPointer(size_t index) const {
    return callStack.front().localPointers[index];
}
void VM::setGlobalPointer(size_t index, Object *object) {
    callStack.front().localPointers[index] = object;
}
void VM::pushPointer(Object *obj) {
    if (pointersStackSize + 1 > pointersStackCapacity) {
        pointersStackCapacity *= 2;
        auto newPointersStack = (Object **) realloc(pointersStack, pointersStackCapacity * sizeof(Object *));
        if (newPointersStack == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        }
        pointersStack = newPointersStack;
    }
    pointersStack[pointersStackSize++] = obj;
}
Object *VM::popPointer() {
    return pointersStack[--pointersStackSize];
}
Object *VM::topPointer() const {
    return pointersStack[pointersStackSize - 1];
}
void VM::addObject(Object *obj) {
    objects.push_back(obj);
    if (objects.size() > gcLimit) {
        markAll();
        sweep();
    }
}
void VM::markAll() {
    for (size_t i = 0; i < pointersStackSize; i++)
        pointersStack[i]->marked = true;
    for (auto stackFrame: callStack) {
        for (size_t i = 0; i < stackFrame.localPointersSize; i++) {
            if (stackFrame.localPointers[i] != nullptr)
                stackFrame.localPointers[i]->marked = true;
        }
    }
}
void VM::sweep() {
    std::erase_if(objects, [](Object *obj) {
        if (!obj->marked) {
            delete obj;
            return true;
        } else {
            obj->marked = false;
            return false;
        }
    });
}
void VM::run(const Program &program) {
    if (callStack.front().localsSize != program.segments.front().number_of_locals) {
        callStack.front().localsSize = program.segments.front().number_of_locals;
        auto *newPtr = (uint64_t *) realloc(callStack.front().locals, callStack.front().localsSize * sizeof(uint64_t));
        if (newPtr == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        } else {
            callStack.front().locals = newPtr;
        }
    }
    if (callStack.front().localPointersSize != program.segments.front().number_of_local_ptr) {
        callStack.front().localPointersSize = program.segments.front().number_of_local_ptr;
        auto *newPtr = (Object **) realloc(callStack.front().localPointers, callStack.front().localPointersSize * sizeof(Object *));
        if (newPtr == nullptr) {
            throw std::runtime_error("Memory allocation failure!");
        } else {
            callStack.front().localPointers = newPtr;
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
            case Instruction::StoreGlobalObject: {
                auto val = popPointer();
                setGlobalPointer(instruction.params.index, val);
            } break;
            case Instruction::StoreGlobalI64: {
                auto val = popStack();
                setGlobal(instruction.params.index, val);
            } break;
            case Instruction::StoreLocalObject: {
                auto val = popPointer();
                setPointer(instruction.params.index, val);
            } break;
            case Instruction::StoreLocalI64: {
                auto val = popStack();
                setLocal(instruction.params.index, val);
            } break;
            case Instruction::LoadI64: {
                pushStack(instruction.params.i64);
            } break;
            case Instruction::LoadGlobalObject: {
                auto val = getGlobalPointer(instruction.params.index);
                pushPointer(val);
            } break;
            case Instruction::LoadGlobalI64: {
                auto val = getGlobal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::LoadLocalObject: {
                auto val = getPointer(instruction.params.index);
                pushPointer(val);
            } break;
            case Instruction::LoadLocalI64: {
                auto val = getLocal(instruction.params.index);
                pushStack(val);
            } break;
            case Instruction::LoadObject: {
                pushPointer((Object *) instruction.params.ptr);
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
                pushPointer(newObject);
                addObject(newObject);
            } break;
            case Instruction::LoadFromLocalArray: {
                auto index = popStack();
                auto array = (ArrayObject *) getPointer(instruction.params.index);
                if (index >= array->size) {
                    throw std::runtime_error("[VM::run] Array index out of bounds!");
                }
                pushStack(array->data[index]);
            } break;
            case Instruction::LoadFromGlobalArray: {
                auto index = popStack();
                auto array = (ArrayObject *) getGlobalPointer(instruction.params.index);
                if (index >= array->size) {
                    throw std::runtime_error("[VM::run] Array index out of bounds!");
                }
                pushStack(array->data[index]);
            } break;
            case Instruction::AppendToArray: {
                auto array = (ArrayObject *) popPointer();
                auto val = popStack();
                auto data = (uint64_t *) realloc(array->data, (array->size + 1) * sizeof(uint64_t));
                if (data == nullptr) {
                    throw std::runtime_error("Memory allocation failure!");
                }
                array->data = data;
                array->data[array->size++] = val;
                pushPointer(array);
            } break;
            case Instruction::LoadLib: {
                loadNativeFunction();
            } break;
            case Instruction::CallNative: {
                callNativeFunction();
            } break;
            case Instruction::Exit:
                return;
        }
        callStack.back().currentInstruction++;
    }
}
void VM::callNativeFunction() {
    auto dynamicLib = (DynamicLibObject *) popPointer();
    auto funcInfo = (DynamicFunctionObject *) popPointer();
    auto func = (NativeFunction) dlsym(dynamicLib->handle, funcInfo->name.c_str());
    if (func == nullptr) {
        throw std::runtime_error(dlerror());
    }
    std::vector<uint64_t> args;
    for (auto arg: funcInfo->arguments) {
        switch (arg->type) {
            case VariableType::Bool:
            case VariableType::I64: {
                auto val = popStack();
                args.push_back(val);
            } break;
            case VariableType::Array:
            case VariableType::Object: {
                auto val = popPointer();
                args.push_back(std::bit_cast<uint64_t>(val));
            } break;
            default:
                throw std::runtime_error("[VM::run] Invalid argument type!");
        }
    }
    auto returnValue = func({
            .argc = args.size(),
            .argv = args.data(),
    });
    switch (returnValue.type) {
        case ExternReturn::SPL_VOID:
            break;
        case ExternReturn::SPL_VALUE:
            pushStack(returnValue.value);
            break;
        case ExternReturn::SPL_OBJECT:
            pushPointer(std::bit_cast<Object *>(returnValue.value));
            addObject(std::bit_cast<Object *>(returnValue.value));
            break;
    }
}
void VM::loadNativeFunction() {
    auto libPath = (StringObject *) popPointer();
    auto handle = dlopen(libPath->chars, RTLD_LAZY);
    if (handle == nullptr) {
        throw std::runtime_error("Object file not found: " + std::string(libPath->chars));
    }
    auto lib = new DynamicLibObject(libPath->chars, handle);
    pushPointer(lib);
    addObject(lib);
}
