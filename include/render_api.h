//
// Abstraction over the graphics API. OpenGL in this case.
//

#ifndef RENDER_API_H

#include "core.h"

typedef enum buffer_usage_t {
    BUFFER_USAGE_STATIC,
    BUFFER_USAGE_DYNAMIC,
    BUFFER_USAGE_STREAM,
} buffer_usage_t;

// -- color_t --------------------------------------------------------------------

typedef struct color_t color_t;
struct color_t {
    f32 r, g, b, a;
};

extern color_t color_rgba_f(f32 r, f32 g, f32 b, f32 a);
extern color_t color_rgba_i(u8 r, u8 g, u8 b, u8 a);
extern color_t color_rgba_hex(u32 hex);
extern color_t color_rgb_f(f32 r, f32 g, f32 b);
extern color_t color_rgb_i(u8 r, u8 g, u8 b);

extern color_t color_rgb_hex(u32 hex);
extern color_t color_hsl(f32 hue, f32 saturation, f32 lightness);
extern color_t color_hsv(f32 hue, f32 saturation, f32 value);

#define color_arg(color) (color).r, (color).g, (color).b, (color).a

#define COLOR_WHITE ((color_t) {1.0f, 1.0f, 1.0f, 1.0f})
#define COLOR_BLACK ((color_t) {0.0f, 0.0f, 0.0f, 1.0f})
#define COLOR_RED ((color_t) {1.0f, 0.0f, 0.0f, 1.0f})
#define COLOR_GREEN ((color_t) {0.0f, 1.0f, 0.0f, 1.0f})
#define COLOR_BLUE ((color_t) {0.0f, 0.0f, 1.0f, 1.0f})

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

typedef enum texture_format_t {
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

typedef enum texture_sampler_t {
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
// Holds the target textures for a render pass.

typedef struct framebuffer_t framebuffer_t;
struct framebuffer_t {
    u32 handle;
};

typedef enum framebuffer_attachment_t {
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

// -- Pipeline -----------------------------------------------------------------
// Holds all state needed for the GPU to draw. Things like shaders, blend state
// and vertex layout.

typedef enum vertex_attribute_type_t {
    VERTEX_ATTRIB_TYPE_F32,
} vertex_attribute_type_t;

typedef struct vertex_attribute_t vertex_attribute_t;
struct vertex_attribute_t {
    u32 offset;
    vertex_attribute_type_t type;
    u32 count;
};

typedef struct vertex_layout_t vertex_layout_t;
struct vertex_layout_t {
    u32 stride;
    vertex_attribute_t* attribs;
    u32 attrib_count;
};

typedef struct pipeline_t pipeline_t;
struct pipeline_t {
    shader_t shader;
    vertex_layout_t vertex_layout;
    u32 vao_handle;
};

typedef struct pipeline_desc_t pipeline_desc_t;
struct pipeline_desc_t {
    shader_t shader;
    vertex_layout_t vertex_layout;
};

extern pipeline_t pipeline_create(pipeline_desc_t desc);
extern void pipeline_destroy(pipeline_t pipeline);

// -- Render pass --------------------------------------------------------------

typedef enum load_op_t {
    LOAD_OP_LOAD,
    LOAD_OP_CLEAR,
} load_op_t;

typedef struct render_pass_t render_pass_t;
struct render_pass_t {
    texture_t target;
    load_op_t load_op;
    color_t clear_color;
};

// -- Command buffer -----------------------------------------------------------

typedef enum cmd_type_t {
    CMD_TYPE_RENDER_PASS_BEGIN,
    CMD_TYPE_RENDER_PASS_END,

    CMD_TYPE_BIND_PIPELINE,
    CMD_TYPE_BIND_VERTEX_BUFFER,
    CMD_TYPE_BIND_INDEX_BUFFER,

    CMD_TYPE_DRAW,
    CMD_TYPE_DRAW_INDEXED,
} cmd_type_t;

typedef union cmd_data_t cmd_data_t;
union cmd_data_t {
    // CMD_TYPE_RENDER_PASS_BEGIN
    render_pass_t render_pass;
    // CMD_TYPE_BIND_PIPELINE
    pipeline_t pipeline;
    // CMD_TYPE_BIND_VERTEX_BUFFER
    vertex_buffer_t vertex_buffer;
    // CMD_TYPE_BIND_INDEX_BUFFER
    index_buffer_t index_buffer;
    // CMD_TYPE_DRAW
    // CMD_TYPE_INDEXED
    struct {
        u32 count;
        u32 first_offset;
    } draw;
};

typedef struct cmd_t cmd_t;
struct cmd_t {
    cmd_t *next;
    cmd_t *prev;
    cmd_type_t type;
    cmd_data_t data;
};

typedef struct cmd_list_t cmd_list_t;
struct cmd_list_t {
    cmd_t *first;
    cmd_t *last;
};

typedef struct cmd_buffer_t cmd_buffer_t;
struct cmd_buffer_t {
    arena_t* arena;
    cmd_list_t list;

    b8 pipeline_bound;
    pipeline_t current_pipeline;

    b8 vertex_buffer_bound;

    b8 index_buffer_bound;
    index_buffer_t current_index_buffer;
};

extern cmd_buffer_t cmd_buffer_init(arena_t* arena);

extern void cmd_buffer_begin(cmd_buffer_t* buffer);
extern void cmd_buffer_end(cmd_buffer_t* buffer);
extern void cmd_buffer_submit(cmd_buffer_t* buffer);

extern void cmd_render_pass_begin(cmd_buffer_t* buffer, render_pass_t pass);
extern void cmd_render_pass_end(cmd_buffer_t* buffer);

extern void cmd_bind_pipeline(cmd_buffer_t* buffer, pipeline_t pipeline);
extern void cmd_bind_vertex_buffer(cmd_buffer_t* buffer, vertex_buffer_t vertex_buffer);
extern void cmd_bind_index_buffer(cmd_buffer_t* buffer, index_buffer_t index_buffer);

extern void cmd_draw(cmd_buffer_t* buffer, u32 vertex_count, u32 first_vertex);
extern void cmd_draw_indexed(cmd_buffer_t* buffer, u32 index_count, u32 first_index);

#endif // RENDER_API_H
