#pragma once

#include <cstdint>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

struct Instruction {
    enum InstructionType : uint8_t {
        Invalid = 0,
        AddI32,
        AddI64,
        SubI32,
        SubI64,
        MulI32,
        MulI64,
        DivI32,
        DivI64,
        ModI32,
        ModI64,
        GreaterI32,
        GreaterI64,
        LessI32,
        LessI64,
        GreaterEqualI32,
        GreaterEqualI64,
        LessEqualI32,
        LessEqualI64,
        EqualI32,
        EqualI64,
        NotEqualI32,
        NotEqualI64,
        IncrementU32,
        IncrementI32,
        IncrementI64,
        DecrementU32,
        DecrementI32,
        DecrementI64,
        StoreGlobalI32,
        StoreGlobalI64,
        StoreGlobalU32,
        StoreLocalI32,
        StoreLocalI64,
        StoreLocalU32,
        LoadI32,
        LoadU32,
        LoadI64,
        LoadGlobalI32,
        LoadGlobalI64,
        LoadGlobalU32,
        LoadLocalI32,
        LoadLocalI64,
        LoadLocalU32,
        ConvertI32toI64,
        ConvertU32toI64,
        Return,
        Call,
        JumpIfFalse,
        Jump,
    } type{};
    union {
        void *ptr;
        size_t index;
        int32_t i32;
        int64_t i64;
        uint32_t u32;
        struct {
            size_t index;
            int32_t i32;
        } ri32;
        struct {
            size_t index;
            int64_t i64;
        } ri64;
        struct {
            size_t index;
            uint32_t u32;
        } ru64;
    } params{};
};

struct VariableType {
    enum Type {
        Invalid = 0,
        Bool,
        I32,
        I64,
        U32,
        Function
    } type;
    explicit VariableType(Type type) : type(type){};
};

struct FunctionType : public VariableType {
    VariableType *returnType;
    std::vector<VariableType *> arguments;
    FunctionType(VariableType *returnType, std::vector<VariableType *> arguments)
        : VariableType(Function), returnType(returnType), arguments(std::move(arguments)){};
};

struct Variable {
    std::string name;
    VariableType *type{};
    size_t index{};
    size_t size{};
    Variable() = default;
    Variable(std::string name, VariableType *type, size_t index, size_t size)
        : name(std::move(name)), type(type), index(index), size(size){};
};

struct Segment {
    std::vector<Instruction> instructions;
    std::unordered_map<std::string, Variable> locals;
    std::unordered_map<std::string, Variable> functions;
    size_t locals_capacity;
    size_t id{};
    size_t find_local(const std::string &identifier);
    void declare_variable(const std::string &name, VariableType *varType);
    void declare_function(const std::string &name, VariableType *funcType, size_t index);
};

struct Program {
    std::vector<Segment> segments;
    Program();
    size_t find_global(const std::string &identifier);
    Variable find_function(const Segment &segment, const std::string &identifier);
};

struct StackFrame {
    uint32_t *locals{};
    size_t localsSize{};
    size_t segmentIndex{};
    size_t currentInstruction{};
};

class VM {
    uint32_t *stack;
    size_t stackCapacity;
    std::vector<StackFrame> callStack;

public:
    VM();
    void newStackFrame(const Segment &segment, size_t id);
    void popStackFrame();
    uint32_t getLocal(size_t index);
    void setLocal(size_t index, uint32_t value);
    uint32_t getGlobal(size_t index);
    void setGlobal(size_t index, uint32_t value);
    uint64_t getDoubleLocal(size_t index);
    void setDoubleLocal(size_t index, uint64_t value);
    uint64_t getDoubleGlobal(size_t index);
    void setDoubleGlobal(size_t index, uint64_t value);
    void pushStack(uint32_t value);
    void pushDoubleStack(uint64_t value);
    uint32_t popStack();
    uint64_t popDoubleStack();
    uint32_t topStack();
    [[maybe_unused]] [[maybe_unused]] uint64_t topDoubleStack();

    void run(const Program &program);
    size_t stackSize{};
};
