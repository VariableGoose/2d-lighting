#include "core.h"
#include "program.h"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

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
    u32 vbo;
    u32 ibo;
    shader_t shader;
};

model_t setup_triangle(void) {
    str_t vert = str_lit(
            "#version 300 es\n"
            "layout (location = 0) in vec2 a_pos;\n"
            "void main() {\n"
            "    gl_Position = vec4(a_pos, 0.0, 1.0);\n"
            "}\0"
        );
    str_t frag = str_lit(
            "#version 300 es\n"
            "#ifdef GL_ES\n"
            "precision mediump float;\n"
            "#endif\n"
            "layout (location = 0) out vec4 frag_color;\n"
            "void main() {\n"
            "    frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
            "}\0"
        );
    shader_t shader = shader_new(vert, frag);

    f32 verts[] = {
        -0.5f, -0.5f,
         0.5f, -0.5f,
         0.0f,  0.5f,
    };

    u32 indices[] = {
        0, 1, 2,
    };

    u32 vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    u32 ibo = 0;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    u32 vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    // Bind vbo to vao
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

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
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.ibo);

    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, NULL);

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
    renderer_t* rend = renderer_new(800, 600, "What");
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
