#pragma once

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

typedef struct {
    uint64_t argc;
    uint64_t *argv;
} ExternArgs;

typedef struct {
    uint64_t value;
    enum {
        SPL_VOID = 0,
        SPL_VALUE,
        SPL_OBJECT,
    } type;
} ExternReturn;

typedef ExternReturn (*NativeFunction)(ExternArgs);
