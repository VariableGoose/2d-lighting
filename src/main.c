#include "core.h"
#include "program.h"
#include "render_api.h"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

static arena_t* arena = NULL;
static arena_t* frame_arena = NULL;

typedef struct quad_t quad_t;
struct quad_t {
    vertex_buffer_t vbo;
    index_buffer_t ibo;
    shader_t shader;
    pipeline_t pipeline;
};

typedef struct vertex_t vertex_t;
struct vertex_t {
    f32 pos[3];
    color_t color;
    f32 uv[2];
};

quad_t quad_setup(void) {
    f32 verts[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
    };
    u32 indices[] = {
        0, 1, 2,
        2, 3, 1,
    };

    vertex_buffer_t vbo = vertex_buffer_create(verts, sizeof(verts), BUFFER_USAGE_STATIC);
    index_buffer_t ibo = index_buffer_create(indices, arr_len(indices), BUFFER_USAGE_STATIC);

    str_t vert = str_read_file(arena, str_lit("assets/shaders/screen_quad.vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/screen_quad.frag.glsl"));
    shader_t shader = shader_create(vert, frag);

    pipeline_t pipeline = pipeline_create((pipeline_desc_t) {
            .vertex_buffer = vbo,
            .shader = shader,
            .vertex_layout = {
                .stride = sizeof(f32)*4,
                .attribs = (vertex_attribute_t[]) {
                    // Position
                    {
                        .offset = 0 * sizeof(f32),
                        .type = VERTEX_ATTRIB_TYPE_F32,
                        .count = 2,
                    },
                    // UV
                    {
                        .offset = 2 * sizeof(f32),
                        .type = VERTEX_ATTRIB_TYPE_F32,
                        .count = 2,
                    },
                },
                .attrib_count = 2,
            },
        });

    return (quad_t) {
        .pipeline = pipeline,
        .vbo = vbo,
        .ibo = ibo,
        .shader = shader,
    };
}

void quad_draw(quad_t quad, texture_t texture) {
    render_pass_t quad_rp = {
        .load_op = LOAD_OP_LOAD,
        .target = {0},
    };
    render_pass_begin(&quad_rp);

    texture_bind(texture, 0);
    pipeline_bind(quad.pipeline);
    index_buffer_bind(quad.ibo);
    draw_indexed(quad.ibo.count, 0);

    render_pass_end(&quad_rp);
}

typedef struct state_t state_t;
struct state_t {
    texture_t screen_texture;
    framebuffer_t fb;

    vertex_buffer_t vbo;
    index_buffer_t ibo;
    texture_t texture;
    shader_t shader;
    pipeline_t pipeline;
    render_pass_t render_pass;

    quad_t screen_quad;
};

state_t setup_state(void) {
    texture_t screen_texture = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_U8,
            .sampler = TEXTURE_SAMPLER_NEAREST,
        });
    framebuffer_t fb = framebuffer_create();
    framebuffer_attach(fb, FRAMEBUFFER_ATTACHMENT_COLOR, 0, screen_texture);

    vertex_t verts[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    };
    u32 indices[] = {
        0, 1, 2,
        2, 3, 1,
    };

    vertex_buffer_t vbo = vertex_buffer_create(verts, sizeof(verts), BUFFER_USAGE_STATIC);
    index_buffer_t ibo = index_buffer_create(indices, arr_len(indices), BUFFER_USAGE_STATIC);

    str_t vert = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/frag.glsl"));
    shader_t shader = shader_create(vert, frag);

    texture_t texture = texture_create((texture_desc_t) {
            .data = (u8[]) {
                255, 0, 0, 128,
                0, 255, 0, 128,
                0, 0, 255, 128,
                255, 0, 255, 128,
            },
            .width = 2,
            .height = 2,
            .sampler = TEXTURE_SAMPLER_NEAREST,
            .format = TEXTURE_FORMAT_RGBA_U8,
        });

    vertex_layout_t layout = {
        .attribs = (vertex_attribute_t[]) {
            // Position
            {
                .offset = offset(vertex_t, pos),
                .type = VERTEX_ATTRIB_TYPE_F32,
                .count = 3,
            },
            // Color
            {
                .offset = offset(vertex_t, color),
                .type = VERTEX_ATTRIB_TYPE_F32,
                .count = 4,
            },
            // UV
            {
                .offset = offset(vertex_t, uv),
                .type = VERTEX_ATTRIB_TYPE_F32,
                .count = 2,
            },
        },
        .attrib_count = 3,
        .stride = sizeof(vertex_t),
    };

    pipeline_t pipeline = pipeline_create((pipeline_desc_t) {
            .vertex_layout = layout,
            .shader = shader,
            .vertex_buffer = vbo,
            .blend = {
                .enabled = true,
                .color_op = BLEND_OP_ADD,
                .src_color_factor = BLEND_FACTOR_SRC_ALPHA,
                .dst_color_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .alpha_op = BLEND_OP_ADD,
                .src_alpha_factor = BLEND_FACTOR_ONE,
                .dst_alpha_factor = BLEND_FACTOR_ZERO,
            },
        });

    render_pass_t pass = render_pass_create((render_pass_desc_t) {
            .target = screen_texture,
            .load_op = LOAD_OP_CLEAR,
            .clear_color = color_rgb_hex(0x6495ed),
        });

    return (state_t) {
        .fb = fb,
        .screen_texture = screen_texture,

        .vbo = vbo,
        .ibo = ibo,
        .texture = texture,
        .shader = shader,
        .render_pass = pass,

        .pipeline = pipeline,
        .screen_quad = quad_setup(),
    };
}

void resize_cb(renderer_t* renderer, i32 width, i32 height) {
    (void) renderer;
    printf("Resize: %dx%d\n", width, height);
    glViewport(0, 0, width, height);
}

void update(renderer_t* rend) {
    state_t* state = rend->user_ptr;
    arena_clear(frame_arena);

    render_pass_begin(&state->render_pass);

    texture_bind(state->texture, 0);
    pipeline_bind(state->pipeline);
    index_buffer_bind(state->ibo);
    draw_indexed(state->ibo.count, 0);

    render_pass_end(&state->render_pass);

    quad_draw(state->screen_quad, state->screen_texture);

    renderer_swap_buffers(rend);
}

i32 main(void) {
    arena = arena_new(4<<20);
    frame_arena = arena_new(4<<20);

    renderer_t* rend = renderer_new(800, 600, "Cross-platform rendering");
    rend->resize_cb = resize_cb;
    rend->update_cb = update;

    const u8 *version = glGetString(GL_VERSION);
    printf("%s\n", version);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    state_t state = setup_state();
    rend->user_ptr = &state;

    renderer_run(rend);

    // Cleanup
    renderer_free(rend);

    return 0;
}
