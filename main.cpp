#include "ast.h"
#include "linenoise.h"
#include "utils.h"
#include <fstream>
#include <iostream>

void printTopStack(const VM &vm, const Program &program) {
    if (vm.stackSize == 0) return;
    auto last_instruction = program.segments[0].instructions.back();
    auto type = getInstructionType(program, last_instruction);
    switch (type) {
        case VariableType::Bool:
            std::cout << (vm.topStack().value == 0 ? "false" : "true") << std::endl;
            break;
        case VariableType::I32:
            std::cout << (int32_t) vm.topStack().value << std::endl;
            break;
        case VariableType::I64: {
            std::cout << vm.topDoubleStack().value << std::endl;
        } break;
        case VariableType::U32:
            std::cout << (uint32_t) vm.topStack().value << std::endl;
            break;
        case VariableType::Object: {
            auto stackObj = vm.topDoubleStack();
            auto obj = reinterpret_cast<Object *>(vm.topDoubleStack().value);
            if (obj->objType == Object::Type::String) {
                std::cout << stackObj.asString() << std::endl;
            }
        }
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
