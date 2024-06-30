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
        AddI32_RI,
        AddI32_GI,
        SubI32,
        SubI32_RI,
        SubI32_GI,
        MulI32,
        MulI32_RI,
        MulI32_GI,
        DivI32,
        DivI32_RI,
        DivI32_GI,
        ModI32,
        ModI32_RI,
        ModI32_GI,
        LoadI32,
        GreaterI32,
        GreaterI32_RI,
        GreaterI32_GI,
        LessI32,
        LessI32_RI,
        LessI32_GI,
        GreaterEqualI32,
        LessEqualI32,
        EqualI32,
        NotEqualI32,
        IncrementI32,
        DecrementI32,
        StoreGlobalI32,
        StoreLocalI32,
        LoadGlobalI32,
        LoadLocalI32,
        Return,
        Call,
        JumpIfFalse,
        Jump,
    } type{};
    union {
        void *ptr;
        size_t index;
        int32_t i32;
        struct {
            size_t index;
            int32_t i32;
        } ri;
    } params{};
};

struct Variable {
    std::string name;
    enum class Type {
        Invalid = 0,
        I32,
        Function
    } type;
    size_t index;
};

struct Segment {
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, Variable> locals;
    std::unordered_map<std::string, size_t> functions;
    size_t id{};
    size_t find_local(const std::string &identifier);
    void declare_variable(const std::string &name, Variable::Type type);
    void declare_function(const std::string &name, size_t index);
};

struct Program {
    std::vector<Segment> segments;
    Program();
    size_t find_global(const std::string &identifier);
    size_t find_function(const Segment &segment, const std::string &identifier);
};

struct StackFrame {
    void **locals{};
    size_t number_of_locals{};
    size_t segmentIndex{};
    size_t currentInstruction{};
};

class VM {
    void *stack;
    size_t stackSize{};
    size_t stackCapacity;
    std::vector<StackFrame> callStack;

public:
    VM();
    void newStackFrame(const Segment &segment, size_t id);
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
