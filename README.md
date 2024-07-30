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
define x : int = 42;
define y = 42;
```
### Arrays
```c
define arr : int[] = [1, 2, 3, 4, 5];
define element = arr[0];
```
### Function declaration
```c
define add : function(x : int, y : int) -> int = {
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
