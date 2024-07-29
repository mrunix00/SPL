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
        AddI64,
        SubI64,
        MulI64,
        DivI64,
        ModI64,
        GreaterI64,
        LessI64,
        GreaterEqualI64,
        LessEqualI64,
        EqualI64,
        NotEqualI64,
        IncrementI64,
        DecrementI64,
        StoreGlobalI64,
        StoreGlobalObject,
        StoreLocalI64,
        StoreLocalObject,
        LoadI64,
        LoadObject,
        LoadGlobalI64,
        LoadGlobalObject,
        LoadLocalI64,
        LoadLocalObject,
        Return,
        Call,
        JumpIfFalse,
        Jump,
        Exit
    } type{};
    union {
        void *ptr;
        size_t index;
        int64_t i64;
    } params{};
};

struct VariableType {
    enum Type : uint32_t {
        Invalid = 0,
        Bool,
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
    Variable() = default;
    Variable(std::string name, VariableType *type, size_t index)
        : name(std::move(name)), type(type), index(index){};
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
    uint64_t *locals{};
    size_t localsSize{};
    size_t segmentIndex{};
    size_t currentInstruction{};
};

class VM {
    uint64_t *stack;
    size_t stackCapacity;
    std::vector<StackFrame> callStack;

public:
    VM();
    ~VM();
    void newStackFrame(const Segment &segment);
    void popStackFrame();
    [[nodiscard]] uint64_t getLocal(size_t index) const;
    void setLocal(size_t index, uint64_t value);
    [[nodiscard]] uint64_t getGlobal(size_t index) const;
    void setGlobal(size_t index, uint64_t value);
    void pushStack(uint64_t value);
    uint64_t popStack();
    [[nodiscard]] uint64_t topStack() const;

    void run(const Program &program);
    size_t stackSize{};
};
