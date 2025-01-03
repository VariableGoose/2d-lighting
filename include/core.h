#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;

typedef u8 b8;
typedef u32 b32;

#ifndef true
#define true ((b8) 1)
#endif // true

#ifndef false
#define false ((b8) 0)
#endif // false

#define arr_len(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

#define min(A, B) ((A) > (B) ? (B) : (A))
#define max(A, B) ((A) > (B) ? (A) : (B))

// -- Arena --------------------------------------------------------------------
// Linear allocator

typedef struct arena_t arena_t;

extern arena_t* arena_new(u32 capacity);
extern void arena_free(arena_t* arena);

extern void* arena_push(arena_t* arena, u32 size);
extern void arena_pop(arena_t* arena, u32 size);
extern void arena_clear(arena_t* arena);

// -- String -------------------------------------------------------------------
// Length based strings.

typedef struct str_t str_t;
struct str_t {
    const u8* data;
    u32 len;
};

extern str_t str(const u8* data, u32 len);

#define str_cstr(CSTR) str((const u8*) (CSTR), strlen(CSTR))
#define str_lit(LIT) str((const u8*) (LIT), sizeof(LIT) - 1)
// Used to pass strings into 'printf' and other formatting functions with
// '%.*s'.
#define str_arg(STR) (i32) (STR).len, (STR).data

extern const char* str_to_cstr(arena_t* arena, str_t str);
extern str_t str_read_file(arena_t* arena, str_t filename);

#endif // CORE_H
