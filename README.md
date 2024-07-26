# SPL
Simple programming language (WIP).

<img src="assets/logo.svg" width=50%>

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
```

### Build
```console
cd SPL/
make -j`nproc`
./build/SPL
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
