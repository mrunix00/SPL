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
        IncrementU32,
        IncrementI32,
        IncrementI64,
        DecrementU32,
        DecrementI32,
        DecrementI64,
        StoreGlobalI32,
        StoreGlobalI64,
        StoreGlobalU32,
        StoreGlobalObject,
        StoreLocalI32,
        StoreLocalI64,
        StoreLocalU32,
        StoreLocalObject,
        LoadI32,
        LoadU32,
        LoadI64,
        LoadObject,
        LoadGlobalI32,
        LoadGlobalI64,
        LoadGlobalU32,
        LoadGlobalObject,
        LoadLocalI32,
        LoadLocalI64,
        LoadLocalU32,
        LoadLocalObject,
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
        } ru32;
    } params{};
};

struct VariableType {
    enum Type : uint32_t {
        Invalid = 0,
        Bool,
        I32,
        I64,
        U32,
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

struct alignas(uint32_t) StackObject {
    VariableType::Type type;
    uint32_t value;

    bool operator==(const StackObject &other) const {
        return type == other.type && value == other.value;
    }
    bool operator==(int32_t other) const {
        return value == other;
    }
};

struct DoubleStackObject {
    VariableType::Type type;
    uint64_t value;

    char *asString() const {
        if (type != VariableType::Object) throw std::runtime_error("Invalid string!");
        auto obj = reinterpret_cast<Object *>(value);
        if (obj->objType != Object::Type::String) throw std::runtime_error("Invalid string!");
        auto str = static_cast<StringObject *>(obj);
        return str->chars;
    }
    bool operator==(const StackObject &other) const {
        return type == other.type && value == other.value;
    }
    bool operator==(int64_t other) const {
        return value == other;
    }
    bool operator==(const char *other) const {
        if (type != VariableType::Object) return false;
        auto obj = reinterpret_cast<Object *>(value);
        if (obj->objType != Object::Type::String) return false;
        auto str = static_cast<StringObject *>(obj);
        return str->length == strlen(other) && !strcmp(str->chars, other);
    }
};

struct StackFrame {
    void *locals{};
    size_t localsSize{};
    size_t segmentIndex{};
    size_t currentInstruction{};
};

class VM {
    void *stack;
    size_t stackCapacity;
    std::vector<StackFrame> callStack;

public:
    VM();
    void newStackFrame(const Segment &segment, size_t id);
    void popStackFrame();
    StackObject getLocal(size_t index);
    void setLocal(size_t index, StackObject value);
    StackObject getGlobal(size_t index);
    void setGlobal(size_t index, StackObject value);
    DoubleStackObject getDoubleLocal(size_t index);
    void setDoubleLocal(size_t index, DoubleStackObject value);
    DoubleStackObject getDoubleGlobal(size_t index);
    void setDoubleGlobal(size_t index, DoubleStackObject value);
    void pushStack(StackObject value);
    void pushDoubleStack(DoubleStackObject value);
    StackObject popStack();
    DoubleStackObject popDoubleStack();
    StackObject topStack();
    [[maybe_unused]] [[maybe_unused]] DoubleStackObject topDoubleStack();

    void run(const Program &program);
    size_t stackSize{};
};
