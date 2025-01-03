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

typedef struct shader_t shader_t;
struct shader_t {
    u32 handle;
};

shader_t shader_new(str_t vertex_source, str_t fragment_source) {
    i32 success = 0;
    char info_log[512] = {0};

    u32 v_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v_shader, 1, (const char* const*) &vertex_source.data, (const int*) &vertex_source.len);
    glCompileShader(v_shader);
    glGetShaderiv(v_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(v_shader, sizeof(info_log), NULL, info_log);
        printf("Vertex shader compilation error: %s\n", info_log);
        return (shader_t) {0};
    }

    u32 f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f_shader, 1, (const char* const*) &fragment_source.data, (const int*) &fragment_source.len);
    glCompileShader(f_shader);
    glGetShaderiv(f_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(f_shader, sizeof(info_log), NULL, info_log);
        printf("Vertex shader compilation error: %s\n", info_log);
        return (shader_t) {0};
    }

    u32 program = glCreateProgram();
    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("Shader linking error: %s\n", info_log);
        return (shader_t) {0};
    }

    glDeleteShader(v_shader);
    glDeleteShader(f_shader);

    return (shader_t) { program };
}

typedef struct model_t model_t;
struct model_t {
    u32 vao;
    vertex_buffer_t vbo;
    index_buffer_t ibo;
    shader_t shader;
};

model_t setup_triangle(void) {
    str_t vert = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    str_t frag = str_read_file(arena, str_lit("assets/shaders/frag.glsl"));
    shader_t shader = shader_new(vert, frag);

    f32 verts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
        -0.5f,  0.5f,
         0.5f,  0.5f,
    };
    vertex_buffer_t vbo = vertex_buffer_create(verts, sizeof(verts), BUFFER_USAGE_STATIC);

    u32 indices[] = {
        0, 1, 2,
        2, 3, 1,
    };
    index_buffer_t ibo = index_buffer_create(indices, arr_len(indices), BUFFER_USAGE_STATIC);

    u32 vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Bind vbo to vao
    vertex_buffer_bind(vbo);

    // Vertex layout
    glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * sizeof(f32), (const void*) 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return (model_t) {
        .vao = vao,
        .vbo = vbo,
        .ibo = ibo,
        .shader = shader,
    };
}

void draw_model(model_t model) {
    glUseProgram(model.shader.handle);

    glBindVertexArray(model.vao);
    index_buffer_bind(model.ibo);

    glDrawElements(GL_TRIANGLES, model.ibo.count, GL_UNSIGNED_INT, NULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
