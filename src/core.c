#include "core.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// -- Arena --------------------------------------------------------------------
// Linear allocator
// :arena

// NOTE: Temporary implementation. Supposed to be a platform dependant
// implementation using mmap. It also lacks alignment and all that fun stuff.

struct arena_t {
    u8* data;
    u32 cap;
    u32 pos;
};

arena_t* arena_new(u32 capacity) {
    arena_t* arena = malloc(sizeof(arena_t) + capacity);
    *arena = (arena_t) {
        .data = (u8*) arena + sizeof(arena_t),
        .cap = capacity,
        .pos = 0,
    };
    return arena;
}

void arena_free(arena_t* arena) {
    free(arena);
}

void* arena_push(arena_t* arena, u32 size) {
    void* ptr = arena->data + arena->pos;
    arena->pos += size;
    return ptr;
}

void arena_pop(arena_t* arena, u32 size) {
    arena->pos -= min(arena->pos, size);
}

void arena_clear(arena_t* arena) {
    arena->pos = 0;
}

// -- String -------------------------------------------------------------------
// Length based strings.
// :string

str_t str(const u8* data, u32 len) {
    return (str_t) { data, len };
}

const char* str_to_cstr(arena_t* arena, str_t str) {
    char* cstr = arena_push(arena, str.len + 1);
    memcpy(cstr, str.data, str.len);
    cstr[str.len] = 0;
    return cstr;
}

str_t str_read_file(arena_t* arena, str_t filename) {
    const char* cstr_filename = str_to_cstr(arena, filename);
    FILE *fp = fopen(cstr_filename, "rb");
    // Pop off the cstr_filename from the arena since it's no longer needed.
    arena_pop(arena, filename.len + 1);
    if (fp == NULL) {
        printf("ERROR: Failed to open file '%.*s'.\n", str_arg(filename));
        return (str_t) {0};
    }

    fseek(fp, 0, SEEK_END);
    u32 len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    u8* content = arena_push(arena, len);
    fread(content, sizeof(u8), len, fp);

    return str(content, len);
}
