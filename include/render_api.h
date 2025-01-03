//
// Abstraction over the graphics API. OpenGL in this case.
//

#ifndef RENDER_API_H

#include "core.h"

typedef enum buffer_usage_t{
    BUFFER_USAGE_STATIC,
    BUFFER_USAGE_DYNAMIC,
    BUFFER_USAGE_STREAM,
} buffer_usage_t;

// -- Vertex buffer ------------------------------------------------------------

typedef struct vertex_buffer_t vertex_buffer_t;
struct vertex_buffer_t {
    u32 handle;
    u32 size;
};

extern vertex_buffer_t vertex_buffer_create(const void* data, u32 size, buffer_usage_t usage);
extern void vertex_buffer_destroy(vertex_buffer_t buffer);
extern void vertex_buffer_bind(vertex_buffer_t buffer);
extern void vertex_buffer_unbind(void);

// -- Index buffer -------------------------------------------------------------

typedef struct index_buffer_t index_buffer_t;
struct index_buffer_t {
    u32 handle;
    u32 count;
};

extern index_buffer_t index_buffer_create(const u32* data, u32 count, buffer_usage_t usage);
extern void index_buffer_destroy(index_buffer_t buffer);
extern void index_buffer_bind(index_buffer_t buffer);
extern void index_buffer_unbind(void);

// -- Shader -------------------------------------------------------------------

typedef struct shader_t shader_t;
struct shader_t {
    u32 handle;
};

extern shader_t shader_create(str_t vertex_source, str_t fragment_source);
extern void shader_destroy(shader_t shader);
extern void shader_use(shader_t shader);

#endif // RENDER_API_H
