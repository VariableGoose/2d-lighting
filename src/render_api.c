#include "render_api.h"

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

// -- Vertex buffer ------------------------------------------------------------

vertex_buffer_t vertex_buffer_create(const void* data, u32 size, buffer_usage_t usage) {
    GLint gl_usage;

    switch (usage) {
        case BUFFER_USAGE_STATIC:
            gl_usage = GL_STATIC_DRAW;
            break;
        case BUFFER_USAGE_DYNAMIC:
            gl_usage = GL_DYNAMIC_DRAW;
            break;
        case BUFFER_USAGE_STREAM:
            gl_usage = GL_STREAM_DRAW;
            break;
    }

    vertex_buffer_t buff = {
        .size = size,
    };
    glGenBuffers(1, &buff.handle);
    vertex_buffer_bind(buff);
    glBufferData(GL_ARRAY_BUFFER, size, data, gl_usage);
    vertex_buffer_unbind();

    return buff;
}

void vertex_buffer_destroy(vertex_buffer_t buffer) {
    glDeleteBuffers(1, &buffer.handle);
}

void vertex_buffer_bind(vertex_buffer_t buffer) {
    glBindBuffer(GL_ARRAY_BUFFER, buffer.handle);
}

void vertex_buffer_unbind(void) {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// -- Index buffer -------------------------------------------------------------

index_buffer_t index_buffer_create(const u32* data, u32 count, buffer_usage_t usage) {
    GLint gl_usage;

    switch (usage) {
        case BUFFER_USAGE_STATIC:
            gl_usage = GL_STATIC_DRAW;
            break;
        case BUFFER_USAGE_DYNAMIC:
            gl_usage = GL_DYNAMIC_DRAW;
            break;
        case BUFFER_USAGE_STREAM:
            gl_usage = GL_STREAM_DRAW;
            break;
    }

    index_buffer_t buff = {
        .count = count,
    };
    glGenBuffers(1, &buff.handle);
    index_buffer_bind(buff);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(u32), data, gl_usage);
    index_buffer_unbind();

    return buff;
}

void index_buffer_destroy(index_buffer_t buffer) {
    glDeleteBuffers(1, &buffer.handle);
}

void index_buffer_bind(index_buffer_t buffer) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer.handle);
}

void index_buffer_unbind(void) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
