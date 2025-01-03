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

#endif // CORE_H
