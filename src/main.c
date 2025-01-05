// TODO:
// - A huge mistake has been made. Just make the damn renderer man. Screw the
// Vulkan stuff. Just make it how you want it.
// TL;DR refactor everything to work how you want it.
// - Render passes
// - Fix texture formats

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
    u32 vao;
    shader_t shader;
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

    u32 vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Bind vbo to vao
    vertex_buffer_bind(vbo);

    // Vertex layout
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(f32), (const void*) (0 * sizeof(f32)));
    glEnableVertexAttribArray(0);
    // UV
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(f32), (const void*) (2 * sizeof(f32)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    str_t vert = str_read_file(arena, str_lit("assets/shaders/screen_quad.vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/screen_quad.frag.glsl"));
    shader_t shader = shader_create(vert, frag);

    return (quad_t) {
        .vbo = vbo,
        .ibo = ibo,
        .vao = vao,
        .shader = shader,
    };
}

void quad_draw(quad_t quad, texture_t texture) {
    texture_bind(texture, 0);
    shader_use(quad.shader);
    glBindVertexArray(quad.vao);
    index_buffer_bind(quad.ibo);
    glDrawElements(GL_TRIANGLES, quad.ibo.count, GL_UNSIGNED_INT, NULL);
}

typedef struct state_t state_t;
struct state_t {
    texture_t screen_texture;
    framebuffer_t fb;

    u32 vao;
    vertex_buffer_t vbo;
    index_buffer_t ibo;
    texture_t texture;
    shader_t shader;
    pipeline_t pipeline;

    quad_t screen_quad;
};

state_t setup_state(void) {
    texture_t screen_texture = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_U8,
            .sampler = TEXTURE_SAMPLER_LINEAR,
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

    u32 vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Bind vbo to vao
    vertex_buffer_bind(vbo);

    // Vertex layout
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(vertex_t), (const void*) offset(vertex_t, pos));
    glEnableVertexAttribArray(0);
    // Color
    glVertexAttribPointer(1, 4, GL_FLOAT, false, sizeof(vertex_t), (const void*) offset(vertex_t, color));
    glEnableVertexAttribArray(1);
    // UV
    glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(vertex_t), (const void*) offset(vertex_t, uv));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    str_t vert = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/frag.glsl"));
    shader_t shader = shader_create(vert, frag);

    texture_t texture = texture_create((texture_desc_t) {
            .data = (u8[]) {
                255, 0, 0, 255,
                0, 255, 0, 255,
                0, 0, 255, 255,
                255, 0, 255, 255,
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
        });

    return (state_t) {
        .fb = fb,
        .screen_texture = screen_texture,

        .vao = vao,
        .vbo = vbo,
        .ibo = ibo,
        .texture = texture,
        .shader = shader,

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

    // quad_draw(state->screen_quad, state->texture);
    //
    // renderer_swap_buffers(rend);
    // return;
    arena_clear(frame_arena);

    cmd_buffer_t buff = cmd_buffer_init(frame_arena);
    cmd_buffer_begin(&buff);
    cmd_render_pass_begin(&buff, (render_pass_t) {
            .target = state->screen_texture,
            .load_op = LOAD_OP_LOAD,
            .clear_color = color_rgb_hex(0x6495ed),
        });

    cmd_bind_pipeline(&buff, state->pipeline);
    cmd_bind_vertex_buffer(&buff, state->vbo);
    cmd_bind_index_buffer(&buff, state->ibo);

    cmd_draw_indexed(&buff, state->ibo.count, 0);

    cmd_render_pass_end(&buff);
    cmd_buffer_end(&buff);
    cmd_buffer_submit(&buff);

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

    state_t state = setup_state();
    rend->user_ptr = &state;

    renderer_run(rend);

    // Cleanup
    renderer_free(rend);

    return 0;
}
