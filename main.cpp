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
    try {
        compile(program, input.c_str());
        vm.run(program);
    } catch (std::runtime_error &error) {
        printf("[-] %s\n", error.what());
        return EXIT_FAILURE;
    }
    // TODO: Add handling for other types
    if (vm.stackSize != 0)
        std::cout << vm.topStack() << std::endl;
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
            return EXIT_FAILURE;
        }
        // TODO: Add handling for other types
        if (vm.stackSize != 0)
            std::cout << vm.topStack() << std::endl;
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
