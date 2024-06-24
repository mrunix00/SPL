#include "ast.h"
#include "vm.h"
#include <fstream>
#include <iostream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input>" << std::endl;
        return EXIT_FAILURE;
    }
    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }
    std::string input((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    VM vm;
    auto program = compile(input.c_str());
    vm.run(program);
    std::cout << *static_cast<int32_t *>(vm.popStack(sizeof(int32_t))) << std::endl;
    return EXIT_SUCCESS;
}
