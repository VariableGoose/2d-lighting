#include "core.h"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

i32 main(void) {
    printf("Hello, world!\n");

    return 0;
}
