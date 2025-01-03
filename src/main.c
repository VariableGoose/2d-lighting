// TODO:
// - Textures
// - Framebuffers
// - Render passes

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

typedef struct model_t model_t;
struct model_t {
    u32 vao;
    vertex_buffer_t vbo;
    index_buffer_t ibo;
    shader_t shader;
    texture_t texture;
};

typedef struct vertex_t vertex_t;
struct vertex_t {
    f32 pos[3];
    f32 color[4];
    f32 uv[2];
};

model_t setup_triangle(void) {
    str_t vert = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/frag.glsl"));
    shader_t shader = shader_create(vert, frag);

    // Triangle
    vertex_t verts[] = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{ 0.0f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.5f, 0.0f}},
    };
    u32 indices[] = {0, 1, 2};

    // Quad
    // vertex_t verts[] = {
    //     {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    //     {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    //     {{-0.5f,  0.5f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}},
    //     {{ 0.5f,  0.5f}, {1.0f, 0.0f, 1.0f, 1.0f}, {1.0f, 0.0f}},
    // };
    // u32 indices[] = {
    //     0, 1, 2,
    //     2, 3, 1,
    // };

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

    texture_t texture = texture_create((texture_desc_t) {
            .data = (u8[]) {
                255, 0, 0, 255,
                0, 255, 0, 255,
                0, 0, 255, 255,
                255, 0, 255, 255,
            },
            .width = 2,
            .height = 2,
            .format = TEXTURE_FORMAT_RGBA_U8,
            .sampler = TEXTURE_SAMPLER_NEAREST,
        });

    shader_use(shader);
    u32 loc = glGetUniformLocation(shader.handle, "my_texture");
    glUniform1i(loc, 0);

    return (model_t) {
        .vao = vao,
        .vbo = vbo,
        .ibo = ibo,
        .shader = shader,
        .texture = texture,
    };
}

void draw_model(model_t model) {
    texture_bind(model.texture, 0);

    shader_use(model.shader);

    glBindVertexArray(model.vao);
    index_buffer_bind(model.ibo);

    glDrawElements(GL_TRIANGLES, model.ibo.count, GL_UNSIGNED_INT, NULL);
}

void resize_cb(renderer_t* renderer, i32 width, i32 height) {
    (void) renderer;
    printf("Resize: %dx%d\n", width, height);
    glViewport(0, 0, width, height);
}

void update(renderer_t* rend) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(100.0f/0xff, 149.0f/0xff, 237.0f/0xff, 1.0f);

    model_t* model = rend->user_ptr;
    draw_model(*model);

    renderer_swap_buffers(rend);
}

i32 main(void) {
    arena = arena_new(4<<20);

    renderer_t* rend = renderer_new(800, 600, "Cross-platform rendering");
    rend->resize_cb = resize_cb;
    rend->update_cb = update;

    const u8 *version = glGetString(GL_VERSION);
    printf("%s\n", version);

    model_t data = setup_triangle();
    rend->user_ptr = &data;

    renderer_run(rend);

    // Cleanup
    renderer_free(rend);

    return 0;
}
