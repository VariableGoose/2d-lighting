#ifndef CORE_H
#define CORE_H

#include <stdint.h>
#include <string.h>
#include <math.h>

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
#define offset(S, M) ((u64) &(((S*) 0)->M))

// -- Arena --------------------------------------------------------------------
// Linear allocator

typedef struct arena_t arena_t;

extern arena_t* arena_new(u32 capacity);
extern void arena_free(arena_t* arena);

extern void* arena_push(arena_t* arena, u32 size);
extern void arena_pop(arena_t* arena, u32 size);
extern void arena_clear(arena_t* arena);

#define arena_push_type(ARENA, T) arena_push((ARENA), sizeof(T))

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

// -- Math -----------------------------------------------------------

#define clamp(V, A, B) ((V) < (A) ? (A) : (V) > (B) ? (B) : (V))
#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))
#define lerp(A, B, T) ((A) + ((B) - (A)) * (T))
#define sign(V) ((V) > 0 ? 1 : (V) < 0 ? -1 : 0)

#define PI 3.14159265359f
#define rad(DEG) ((DEG)/(2*PI))
#define deg(RAD) ((2*(RAD)/PI))

// Vec2
typedef struct Vec2 Vec2;
struct Vec2 {
    f32 x, y;
};

static inline Vec2 vec2(f32 x, f32 y) { return (Vec2) {x, y}; }
static inline Vec2 vec2s(f32 scaler) { return (Vec2) {scaler, scaler}; }

static inline Vec2 vec2_add(Vec2 a, Vec2 b) { return vec2(a.x+b.x, a.y+b.y); }
static inline Vec2 vec2_sub(Vec2 a, Vec2 b) { return vec2(a.x-b.x, a.y-b.y); }
static inline Vec2 vec2_mul(Vec2 a, Vec2 b) { return vec2(a.x*b.x, a.y*b.y); }
static inline Vec2 vec2_div(Vec2 a, Vec2 b) { return vec2(a.x/b.x, a.y/b.y); }

static inline Vec2 vec2_adds(Vec2 vec, f32 scaler) { return vec2(vec.x+scaler, vec.y+scaler); }
static inline Vec2 vec2_subs(Vec2 vec, f32 scaler) { return vec2(vec.x-scaler, vec.y-scaler); }
static inline Vec2 vec2_muls(Vec2 vec, f32 scaler) { return vec2(vec.x*scaler, vec.y*scaler); }
static inline Vec2 vec2_divs(Vec2 vec, f32 scaler) { return vec2(vec.x/scaler, vec.y/scaler); }

static inline f32 vec2_dot(Vec2 a, Vec2 b) {
    return a.x*b.x + a.y*b.y;
}

static inline f32 vec2_magnitude_squared(Vec2 vec) {
    return vec.x*vec.x+vec.y*vec.y;
}

static inline f32 vec2_magnitude(Vec2 vec) {
    return sqrtf(vec.x*vec.x+vec.y*vec.y);
}

static inline Vec2 vec2_normalized(Vec2 vec) {
    f32 mag = vec2_magnitude(vec);
    if (mag == 0.0f) {
        return vec2s(0.0f);
    }
    f32 inv_mag = 1.0f/mag;
    return vec2_muls(vec, inv_mag);
}

static inline Vec2 vec2_lerp(Vec2 a, Vec2 b, f32 t) {
    return vec2(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

// Ivec2
typedef struct Ivec2 Ivec2;
struct Ivec2 {
    i32 x, y;
};

static inline Ivec2 ivec2(i32 x, i32 y) { return (Ivec2) {x, y}; }
static inline Ivec2 ivec2s(i32 scaler) { return (Ivec2) {scaler, scaler}; }

static inline Ivec2 ivec2_add(Ivec2 a, Ivec2 b) { return ivec2(a.x+b.x, a.y+b.y); }
static inline Ivec2 ivec2_sub(Ivec2 a, Ivec2 b) { return ivec2(a.x-b.x, a.y-b.y); }
static inline Ivec2 ivec2_mul(Ivec2 a, Ivec2 b) { return ivec2(a.x*b.x, a.y*b.y); }
static inline Ivec2 ivec2_div(Ivec2 a, Ivec2 b) { return ivec2(a.x/b.x, a.y/b.y); }

static inline Ivec2 ivec2_adds(Ivec2 vec, i32 scaler) { return ivec2(vec.x+scaler, vec.y+scaler); }
static inline Ivec2 ivec2_subs(Ivec2 vec, i32 scaler) { return ivec2(vec.x-scaler, vec.y-scaler); }
static inline Ivec2 ivec2_muls(Ivec2 vec, i32 scaler) { return ivec2(vec.x*scaler, vec.y*scaler); }
static inline Ivec2 ivec2_divs(Ivec2 vec, i32 scaler) { return ivec2(vec.x/scaler, vec.y/scaler); }

#define vec2_arg(vec) (vec).x, (vec).y

// Vec3
typedef struct Vec3 Vec3;
struct Vec3 {
    f32 x, y, z;
};

static inline Vec3 vec3(f32 x, f32 y, f32 z) { return (Vec3) {x, y, z}; }
static inline Vec3 vec3s(f32 scaler) { return (Vec3) {scaler, scaler, scaler}; }

#define vec3_arg(vec) (vec).x, (vec).y, (vec).z

// Vec4
typedef struct Vec4 Vec4;
struct Vec4 {
    f32 x, y, z, w;
};

static inline Vec4 vec4(f32 x, f32 y, f32 z, f32 w) { return (Vec4) {x, y, z, w}; }
static inline Vec4 vec4s(f32 scaler) { return (Vec4) {scaler, scaler, scaler, scaler}; }

// Mat4
typedef struct Mat4 Mat4;
struct Mat4 {
    Vec4 a, b, c, d;
};

static const Mat4 MAT4_IDENTITY = {
    {1.0f, 0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f, 0.0f},
    {0.0f, 0.0f, 1.0f, 0.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
};

static inline Vec4 mat4_mul_vec(Mat4 mat, Vec4 vec) {
    return (Vec4) {
        vec.x*mat.a.x + vec.y*mat.a.y + vec.z*mat.a.z + vec.w*mat.a.w,
        vec.x*mat.b.x + vec.y*mat.b.y + vec.z*mat.b.z + vec.w*mat.b.w,
        vec.x*mat.c.x + vec.y*mat.c.y + vec.z*mat.c.z + vec.w*mat.c.w,
        vec.x*mat.d.x + vec.y*mat.d.y + vec.z*mat.d.z + vec.w*mat.d.w,
    };
}

// https://en.wikipedia.org/wiki/Orthographic_projection#Geometry
static inline Mat4 mat4_ortho_projection(f32 left, f32 right, f32 top, f32 bottom, f32 far, f32 near) {
    f32 x = 2.0f / (right - left);
    f32 y = 2.0f / (top - bottom);
    f32 z = -2.0f / (far - near);

    f32 x_off = -(right+left) / (right-left);
    f32 y_off = -(top+bottom) / (top-bottom);
    f32 z_off = -(far+near) / (far-near);

    return (Mat4) {
        {x, 0, 0, x_off},
        {0, y, 0, y_off},
        {0, 0, z, z_off},
        {0, 0, 0, 1},
    };
}

static inline Mat4 mat4_inv_ortho_projection(f32 left, f32 right, f32 top, f32 bottom, f32 far, f32 near) {
    f32 x = (right - left) / 2.0f;
    f32 y = (top - bottom) / 2.0f;
    f32 z = (far - near) / -2.0f;

    f32 x_off = (left+right) / 2.0f;
    f32 y_off = (top+bottom) / 2.0f;
    f32 z_off = -(far+near) / 2.0f;

    return (Mat4) {
        {x, 0, 0, x_off},
        {0, y, 0, y_off},
        {0, 0, z, z_off},
        {0, 0, 0, 1},
    };
}

static inline Mat4 mat4_translate(Mat4 mat, Vec3 translation) {
    Mat4 result = mat;
    result.d.x += translation.x;
    result.d.y += translation.y;
    result.d.z += translation.z;
    return result;
}

static inline Mat4 mat4_scale(Mat4 mat, Vec3 scale) {
    Mat4 result = mat;
    result.a.x *= scale.x;
    result.b.y *= scale.y;
    result.c.z *= scale.z;
    return result;
}

#endif // CORE_H
