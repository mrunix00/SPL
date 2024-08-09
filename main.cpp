#include "ast.h"
#include "linenoise.h"
#include "utils.h"
#include <fstream>
#include <iostream>

void printTopStack(const VM &vm, const Program &program) {
    if (vm.stackSize == 0 && vm.pointersStackSize == 0) return;
    auto number_of_instructions = program.segments[0].instructions.size();
    auto last_instruction = program.segments[0].instructions[number_of_instructions - 2];
    auto type = getInstructionType(program, last_instruction);
    switch (type) {
        case VariableType::Bool:
            std::cout << (vm.topStack() == 0 ? "false" : "true") << std::endl;
            break;
        case VariableType::I64: {
            std::cout << (int64_t) vm.topStack() << std::endl;
        } break;
        case VariableType::F64: {
            std::cout << std::bit_cast<double>(vm.topStack()) << std::endl;
        } break;
        case VariableType::Object: {
            auto obj = vm.topPointer();
            if (obj->objType == Object::Type::String) {
                std::cout << static_cast<StringObject *>(obj)->chars << std::endl;
            } else if (obj->objType == Object::Type::Array) {
                auto array = static_cast<ArrayObject *>(obj);
                std::cout << "[";
                for (size_t i = 0; i < array->size; i++) {
                    std::cout << array->data[i];
                    if (i != array->size - 1) std::cout << ", ";
                }
                std::cout << "]" << std::endl;
            }
        } break;
        default:
            return;
    }
}

int readFile(const char *filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return EXIT_FAILURE;
    }
    std::string input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    VM vm;
    Program program;
    try {
        compile(program, input.c_str());
        vm.run(program);
    } catch (std::runtime_error &error) {
        printf("[-] %s\n", error.what());
        return EXIT_FAILURE;
    }
    printTopStack(vm, program);
    return EXIT_SUCCESS;
}

int repl() {
    VM vm;
    Program program;
    linenoiseInstallWindowChangeHandler();
    for (;;) {
        char *input = linenoise("SPL> ");
        if (input == nullptr) break;
        if (*input == '\0') continue;
        linenoiseHistoryAdd(input);
        try {
            compile(program, input);
            vm.run(program);
        } catch (std::runtime_error &error) {
            printf("[-] %s\n", error.what());
            continue;
        }
        printTopStack(vm, program);
    }
    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    if (argc == 2) {
        return readFile(argv[1]);
    } else {
        return repl();
    }
}
