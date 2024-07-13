# SPL
Simple programming language (WIP).

## Build instructions

### Prerequisite
You need to have the following installed on your system:
- CMake 3.25+.
- A C++ Compiler.
- Git
- Flex
- Bison

### Cloning the repo
```console
git clone https://github.com/mrunix00/SPL.git
cd SPL/
git submodule update --init
```

### Build
```console
mkdir build && cd build
cmake .. && cmake --build .
./SPL
```

## Syntax
### Variable declaration
```c
define x : i32 = 42;
define y = 42;
```
### Function declaration
```c
define add : function(x : i32, y : i32) -> i32 = {
    return x + y;
};
```
### Conditions
```c
if x == 42 {
    // do something
} else {
    // do something else
};
```
### Loops
#### While loops
```c
while x < 42 {
    // do something
};
```
#### For loops
```c
for define i = 0; i < 20; i++ {
    // do something
};
```

## Roadmap
- [x] Lexer
- [x] Parser
- [x] REPL shell
- [x] Variables
- [x] Functions
- [x] Conditions
- [x] While loops
- [x] For loops
- [ ] Arrays
- [ ] Strings
- [ ] Hashmaps
- [ ] Error handling
- [ ] Structs
- [ ] I/O operations
- [x] Bytecode VM
- [ ] Tail call optimization
- [ ] Garbage collector
- [ ] Importing libraries
- [ ] Standard library
- [ ] JIT compiler
