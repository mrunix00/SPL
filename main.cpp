#include "ast.h"
#include "linenoise.h"
#include <fstream>
#include <iostream>

void printTopStack(VM &vm) {
    if (vm.stackSize == 0) return;
    auto top = vm.topStack();
    switch (top.type) {
        case VariableType::Bool:
            std::cout << (top.value == 0 ? "false" : "true") << std::endl;
            break;
        case VariableType::I32:
            std::cout << (int32_t) top.value << std::endl;
            break;
        case VariableType::I64: {
            auto val = vm.topDoubleStack();
            std::cout << val.value << std::endl;
        } break;
        case VariableType::U32:
            std::cout << (uint32_t) top.value << std::endl;
            break;
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
    printTopStack(vm);
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
        printTopStack(vm);
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
