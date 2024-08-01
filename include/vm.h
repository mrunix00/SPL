#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
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
        MakeArray,
        LoadFromLocalArray,
        LoadFromGlobalArray,
        AppendToArray,
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
        Array,
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
struct ArrayObjectType : public VariableType {
    VariableType *elementType;
    explicit ArrayObjectType(VariableType *elementType) : VariableType(Array), elementType(elementType){};
};

struct Object {
    enum class Type {
        Invalid = 0,
        String,
        Array,
    } objType;
    Object() : objType(Type::Invalid){};
    virtual ~Object() = default;
    explicit Object(Type type) : objType(type){};
    bool marked = false;
};

struct StringObject : public Object {
    size_t length;
    char *chars;
    StringObject(size_t length, char *chars) : Object(Type::String), length(length), chars(chars){};
    ~StringObject() override {
        free(chars);
    }
    inline bool operator==(StringObject &other) const {
        if (length != other.length) return false;
        return std::memcmp(chars, other.chars, length) == 0;
    }
    inline bool operator==(const char *other) const {
        if (strlen(other) != length) return false;
        return std::memcmp(chars, other, length) == 0;
    }
};
struct ArrayObject : public Object {
    uint64_t *data;
    size_t size;
    ArrayObject(size_t size, uint64_t *data) : Object(Object::Type::Array), size(size), data(data) {}
    ~ArrayObject() override {
        free(data);
    }
    inline bool operator==(ArrayObject &other) const {
        if (size != other.size) return false;
        for (size_t i = 0; i < size; i++) {
            if (data[i] != other.data[i])
                return false;
        }
        return true;
    }
    inline bool operator==(const std::vector<uint64_t> &other) const {
        if (size != other.size()) return false;
        for (size_t i = 0; i < size; i++) {
            if (data[i] != other[i])
                return false;
        }
        return true;
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
    size_t number_of_locals;
    size_t number_of_local_ptr;
    size_t number_of_args{};
    size_t number_of_arg_ptr{};
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
    Object **localPointers{};
    size_t localsSize{};
    size_t localPointersSize{};
    size_t segmentIndex{};
    size_t currentInstruction{};
};

class VM {
    const size_t gcLimit = 1024;

    uint64_t *stack;
    Object **pointersStack;
    size_t stackCapacity;
    size_t pointersStackCapacity;
    std::vector<StackFrame> callStack;
    std::vector<Object *> objects;

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
    [[nodiscard]] Object *getPointer(size_t index) const;
    void setPointer(size_t index, Object *object);
    [[nodiscard]] Object *getGlobalPointer(size_t index) const;
    void setGlobalPointer(size_t index, Object *object);
    void pushPointer(Object *);
    Object *popPointer();
    [[nodiscard]] Object *topPointer() const;
    void addObject(Object *);
    void markAll();
    void sweep();

    void run(const Program &program);
    size_t stackSize{};
    size_t pointersStackSize{};
};
