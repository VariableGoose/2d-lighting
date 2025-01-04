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

// -- Texture ------------------------------------------------------------------

typedef struct texture_t texture_t;
struct texture_t {
    u32 handle;
};

typedef enum texture_format_t{
    TEXTURE_FORMAT_R_U8 = 1,
    TEXTURE_FORMAT_RG_U8,
    TEXTURE_FORMAT_RGB_U8,
    TEXTURE_FORMAT_RGBA_U8,

    TEXTURE_FORMAT_R_F16,
    TEXTURE_FORMAT_RG_F16,
    TEXTURE_FORMAT_RGB_F16,
    TEXTURE_FORMAT_RGBA_F16,

    TEXTURE_FORMAT_R_F32,
    TEXTURE_FORMAT_RG_F32,
    TEXTURE_FORMAT_RGB_F32,
    TEXTURE_FORMAT_RGBA_F32,
} texture_format_t;

typedef enum texture_sampler_t{
    TEXTURE_SAMPLER_LINEAR,
    TEXTURE_SAMPLER_NEAREST,
} texture_sampler_t;

typedef struct texture_desc_t texture_desc_t;
struct texture_desc_t {
    const void* data;
    u32 width;
    u32 height;
    texture_format_t format;
    texture_sampler_t sampler;
};

extern texture_t texture_create(texture_desc_t desc);
extern void texture_destroy(texture_t texture);
extern void texture_bind(texture_t texture, u32 slot);

// -- Framebuffer --------------------------------------------------------------

typedef struct framebuffer_t framebuffer_t;
struct framebuffer_t {
    u32 handle;
};

typedef enum framebuffer_attachment_t{
    FRAMEBUFFER_ATTACHMENT_COLOR,
    FRAMEBUFFER_ATTACHMENT_DEPTH,
    FRAMEBUFFER_ATTACHMENT_STENCIL,
} framebuffer_attachment_t;

extern framebuffer_t framebuffer_create(void);
extern void framebuffer_destroy(framebuffer_t fb);
extern void framebuffer_bind(framebuffer_t fb);
extern void framebuffer_unbind(void);
// The slot can only be non 0 if the attachment is 'FRAMEBUFFER_ATTACHMENT_COLOR'.
extern void framebuffer_attach(framebuffer_t fb,
        framebuffer_attachment_t attachment,
        u32 slot,
        texture_t texture);

#endif // RENDER_API_H
