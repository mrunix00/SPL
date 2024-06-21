#pragma once

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

struct Instruction {
    enum InstructionType : uint8_t {
        Invalid = 0,
        AddI32,
        SubI32,
        MulI32,
        DivI32,
        ModI32,
        LoadI32,
        StoreGlobalI32
    } type{};
    union {
        void *ptr;
        size_t index;
        int32_t i32;
    } params{};
};

struct Variable {
    std::string name;
    enum class Type {
        Invalid = 0,
        I32
    } type;
    size_t index;
};

struct Segment {
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, Variable> locals;
};

struct Program {
    std::unordered_map<std::string, Variable> globals;
    std::vector<Segment> segments;
    Program();
    void addGlobal(const std::string &name, Variable::Type type);
};

struct StackFrame {
    // TODO: Make this memory space more compact
    std::vector<void *> locals;
};

class VM {
    void *stack;
    std::vector<void *> globals;
    size_t stackSize{};
    size_t stackCapacity;
    std::stack<StackFrame> callStack;

public:
    VM();
    void newStackFrame(const Segment &segment);
    void popStackFrame();
    void *getLocal(size_t index);
    void setLocal(size_t index, void **value);
    void *getGlobal(size_t index);
    void setGlobal(size_t index, void **value);
    void pushStack(void *value, size_t size);
    void *popStack(size_t size);
    void *topStack(size_t size);

    void run(const Program &program);
};
