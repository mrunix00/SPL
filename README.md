# SPL
Another programming language (WIP).

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

## Roadmap
- [x] Lexer
- [x] Parser
- [ ] REPL shell
- [x] Variables
- [x] Functions
- [x] Conditions
- [ ] Loops
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