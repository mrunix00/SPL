#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
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
        IncrementI32,
        IncrementI64,
        DecrementI32,
        DecrementI64,
        StoreGlobalI32,
        StoreGlobalI64,
        StoreGlobalObject,
        StoreLocalI32,
        StoreLocalI64,
        StoreLocalObject,
        LoadI32,
        LoadI64,
        LoadObject,
        LoadGlobalI32,
        LoadGlobalI64,
        LoadGlobalObject,
        LoadLocalI32,
        LoadLocalI64,
        LoadLocalObject,
        ConvertI32toI64,
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
        struct {
            size_t index;
            int32_t i32;
        } ri32;
        struct {
            size_t index;
            int64_t i64;
        } ri64;
    } params{};
};

struct VariableType {
    enum Type : uint32_t {
        Invalid = 0,
        Bool,
        I32,
        I64,
        Object,
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

struct Object {
    enum class Type {
        Invalid = 0,
        String
    } objType;
    Object() : objType(Type::Invalid){};
    explicit Object(Type type) : objType(type){};
};

struct StringObject : public Object {
    size_t length;
    char *chars;
    StringObject(size_t length, char *chars) : Object(Type::String), length(length), chars(chars){};
    inline bool operator==(StringObject &other) const {
        if (objType != other.objType) return false;
        if (length != other.length) return false;
        return std::memcmp(chars, other.chars, length) == 0;
    }
    inline bool operator==(const char *other) const {
        if (strlen(other) != length) return false;
        return std::memcmp(chars, other, length) == 0;
    }
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
    VariableType *returnType{};
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
    ~VM();
    void newStackFrame(const Segment &segment);
    void popStackFrame();
    [[nodiscard]] uint32_t getLocal(size_t index) const;
    void setLocal(size_t index, uint32_t value);
    [[nodiscard]] uint32_t getGlobal(size_t index) const;
    void setGlobal(size_t index, uint32_t value);
    [[nodiscard]] uint64_t getDoubleLocal(size_t index) const;
    void setDoubleLocal(size_t index, uint64_t value);
    [[nodiscard]] uint64_t getDoubleGlobal(size_t index) const;
    void setDoubleGlobal(size_t index, uint64_t value);
    void pushStack(uint32_t value);
    void pushDoubleStack(uint64_t value);
    uint32_t popStack();
    uint64_t popDoubleStack();
    [[nodiscard]] uint32_t topStack() const;
    [[nodiscard]] uint64_t topDoubleStack() const;

    void run(const Program &program);
    size_t stackSize{};
};
