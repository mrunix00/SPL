#include "testlib.h"
#include <cassert>
#include <cstdio>

ExternReturn hello_world(ExternArgs args) {
    assert(args.argc == 0);
    printf("Hello world from C!\n");
    return (ExternReturn){0, ExternReturn::SPL_VOID};
}