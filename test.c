#include "core.h"

#include <stdio.h>

i32 main(void) {
    arena_t* arena = arena_new(4<<20);

    str_t file = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    printf("Content: %.*s\n", str_arg(file));

    arena_free(arena);
    return 0;
}
