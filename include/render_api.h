//
// Abstraction over the graphics API. OpenGL in this case.
//

#ifndef RENDER_API_H
#define RENDER_API_H

#include "core.h"

typedef enum buffer_usage_t {
    BUFFER_USAGE_STATIC,
    BUFFER_USAGE_DYNAMIC,
    BUFFER_USAGE_STREAM,
} buffer_usage_t;

// -- color_t ------------------------------------------------------------------

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
#define COLOR_TRANSPARENT ((color_t) {0.0f, 0.0f, 0.0f, 0.0f})

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

extern void shader_uniform_vec4(shader_t shader, const char* name, Vec4 value);
extern void shader_uniform_mat4(shader_t shader, const char* name, Mat4 value);
extern void shader_uniform_f32(shader_t shader, const char* name, f32 value);
extern void shader_uniform_i32(shader_t shader, const char* name, i32 value);

// -- Texture ------------------------------------------------------------------

typedef struct texture_t texture_t;
struct texture_t {
    u32 handle;
    Ivec2 size;
};

typedef enum texture_format_t {
    // Each pixel row needs to be a multiple of 4 so a 2x2 RGB_U8 texture needs
    // to pad each row of 6 pixels with 2 extra bytes.
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
// Horrible name but idk what else to name it.
extern void texture_resize(texture_t* texture, texture_desc_t desc);

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
// Holds all state needed for the GPU to draw. Things like blend state and
// vertex layout.

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

typedef enum blend_op_t {
    BLEND_OP_ADD,
    BLEND_OP_SUB,
} blend_op_t;

typedef enum blend_factor_t {
    BLEND_FACTOR_ZERO,
    BLEND_FACTOR_ONE,
    BLEND_FACTOR_SRC_ALPHA,
    BLEND_FACTOR_DST_ALPHA,
    BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
} blend_factor_t;

typedef struct blend_state_t blend_state_t;
struct blend_state_t {
    b8 enabled;
    blend_op_t color_op;
    blend_factor_t src_color_factor;
    blend_factor_t dst_color_factor;
    blend_op_t alpha_op;
    blend_factor_t src_alpha_factor;
    blend_factor_t dst_alpha_factor;
};

typedef struct pipeline_desc_t pipeline_desc_t;
struct pipeline_desc_t {
    vertex_layout_t vertex_layout;
    vertex_buffer_t vertex_buffer;
    blend_state_t blend;
};

typedef struct pipeline_t pipeline_t;
struct pipeline_t {
    pipeline_desc_t desc;
    u32 vao_handle;
};

extern pipeline_t pipeline_create(pipeline_desc_t desc);
extern void pipeline_destroy(pipeline_t pipeline);
extern void pipeline_bind(pipeline_t pipeline);

// -- Render pass --------------------------------------------------------------

typedef enum load_op_t {
    LOAD_OP_LOAD,
    LOAD_OP_CLEAR,
} load_op_t;

typedef struct render_pass_desc_t render_pass_desc_t;
struct render_pass_desc_t {
    texture_t targets[32];
    u32 target_count;
    load_op_t load_op;
    color_t clear_color;
};

typedef struct render_pass_t render_pass_t;
struct render_pass_t {
    render_pass_desc_t desc;
    framebuffer_t target_fb;
};

extern render_pass_t render_pass_create(render_pass_desc_t desc);
extern void render_pass_destroy(render_pass_t pass);

extern void render_pass_begin(render_pass_t* pass);
extern void render_pass_end(render_pass_t* pass);
#define RENDER_PASS(PASS) \
    for (b8 _i_ = (render_pass_begin(PASS), false); !_i_; _i_ = true, render_pass_end(PASS))

extern void draw(u32 vertex_count, u32 first_vertex);
extern void draw_indexed(u32 index_count, u32 first_index);

#endif // RENDER_API_H
