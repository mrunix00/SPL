#include "ast.h"
#include "linenoise.h"
#include <fstream>
#include <iostream>

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
    compile(program, input.c_str());
    vm.run(program);
    if (vm.stackSize == sizeof(int32_t))
        std::cout << *static_cast<int32_t *>(vm.topStack(sizeof(int32_t))) << std::endl;
    else if (vm.stackSize == sizeof(int64_t))
        std::cout << *static_cast<int64_t *>(vm.topStack(sizeof(int64_t))) << std::endl;
    return EXIT_SUCCESS;
}

[[noreturn]] int repl() {
    VM vm;
    Program program;
    while (true) {
        char *input = linenoise("SPL> ");
        if (input == nullptr) {
            break;
        }
        linenoiseHistoryAdd(input);
        try {
            compile(program, input);
            vm.run(program);
        } catch (std::runtime_error &error) {
            std::cout << "[-] " << error.what() << '\n';
        }
        // TODO: Add handling for other types
        if (vm.stackSize != 0)
            std::cout << *static_cast<int32_t *>(vm.topStack(sizeof(int32_t))) << std::endl;
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
        return readFile(argv[1]);
    } else {
        return repl();
    }
}
