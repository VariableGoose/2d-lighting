#include "render_api.h"
#include "core.h"

#include <stdio.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

// -- color_t --------------------------------------------------------------------

color_t color_rgba_f(f32 r, f32 g, f32 b, f32 a) {
    return (color_t) {r, g, b, a};
}

color_t color_rgba_i(u8 r, u8 g, u8 b, u8 a) {
    return (color_t) {r/255.0f, g/255.0f, b/255.0f, a/255.0f};
}

color_t color_rgba_hex(u32 hex) {
    return (color_t) {
        .r = (f32) (hex >> 8 * 3 & 0xff) / 0xff,
        .g = (f32) (hex >> 8 * 2 & 0xff) / 0xff,
        .b = (f32) (hex >> 8 * 1 & 0xff) / 0xff,
        .a = (f32) (hex >> 8 * 0 & 0xff) / 0xff,
    };
}

color_t color_rgb_f(f32 r, f32 g, f32 b) {
    return (color_t) {r, g, b, 1.0f};
}

color_t color_rgb_i(u8 r, u8 g, u8 b) {
    return (color_t) {r/255.0f, g/255.0f, b/255.0f, 1.0f};
}

color_t color_rgb_hex(u32 hex) {
    return (color_t) {
        .r = (f32) (hex >> 8 * 2 & 0xff) / 0xff,
        .g = (f32) (hex >> 8 * 1 & 0xff) / 0xff,
        .b = (f32) (hex >> 8 * 0 & 0xff) / 0xff,
        .a = 1.0f,
    };
}

color_t color_hsl(f32 hue, f32 saturation, f32 lightness) {
    // https://en.wikipedia.org/wiki/HSL_and_HSV#HSL_to_RGB
    color_t color = {0};
    f32 chroma = (1 - fabsf(2 * lightness - 1)) * saturation;
    f32 hue_prime = fabsf(fmodf(hue, 360.0f)) / 60.0f;
    f32 x = chroma * (1.0f - fabsf(fmodf(hue_prime, 2.0f) - 1.0f));
    if (hue_prime < 1.0f) { color = (color_t) { chroma, x, 0.0f, 1.0f, }; }
    else if (hue_prime < 2.0f) { color = (color_t) { x, chroma, 0.0f, 1.0f, }; }
    else if (hue_prime < 3.0f) { color = (color_t) { 0.0f, chroma, x, 1.0f, }; }
    else if (hue_prime < 4.0f) { color = (color_t) { 0.0f, x, chroma, 1.0f, }; }
    else if (hue_prime < 5.0f) { color = (color_t) { x, 0.0f, chroma, 1.0f, }; }
    else if (hue_prime < 6.0f) { color = (color_t) { chroma, 0.0f, x, 1.0f, }; }
    f32 m = lightness-chroma / 2.0f;
    color.r += m;
    color.g += m;
    color.b += m;
    return color;
}

color_t color_hsv(f32 hue, f32 saturation, f32 value) {
    // https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB
    color_t color = {0};
    f32 chroma = value * saturation;
    f32 hue_prime = fabsf(fmodf(hue, 360.0f)) / 60.0f;
    f32 x = chroma * (1.0f - fabsf(fmodf(hue_prime, 2.0f) - 1.0f));
    if (hue_prime < 1.0f) { color = (color_t) { chroma, x, 0.0f, 1.0f, }; }
    else if (hue_prime < 2.0f) { color = (color_t) { x, chroma, 0.0f, 1.0f, }; }
    else if (hue_prime < 3.0f) { color = (color_t) { 0.0f, chroma, x, 1.0f, }; }
    else if (hue_prime < 4.0f) { color = (color_t) { 0.0f, x, chroma, 1.0f, }; }
    else if (hue_prime < 5.0f) { color = (color_t) { x, 0.0f, chroma, 1.0f, }; }
    else if (hue_prime < 6.0f) { color = (color_t) { chroma, 0.0f, x, 1.0f, }; }
    f32 m = value - chroma;
    color.r += m;
    color.g += m;
    color.b += m;
    return color;
}

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

// -- Shader -------------------------------------------------------------------

shader_t shader_create(str_t vertex_source, str_t fragment_source) {
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
        printf("Fragment shader compilation error: %s\n", info_log);
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

void shader_destroy(shader_t shader) {
    glDeleteProgram(shader.handle);
}

void shader_use(shader_t shader) {
    glUseProgram(shader.handle);
}

// -- Texture ------------------------------------------------------------------

texture_t texture_create(texture_desc_t desc) {
    u32 gl_internal_format;
    u32 gl_format;
    switch (desc.format) {
        case TEXTURE_FORMAT_R_U8:
            gl_internal_format = GL_R8;
            gl_format = GL_RED;
            break;
        case TEXTURE_FORMAT_RG_U8:
            gl_internal_format = GL_RG8;
            gl_format = GL_RG;
            break;
        case TEXTURE_FORMAT_RGB_U8:
            gl_internal_format = GL_RGB8;
            gl_format = GL_RGB;
            break;
        case TEXTURE_FORMAT_RGBA_U8:
            gl_internal_format = GL_RGBA8;
            gl_format = GL_RGBA;
            break;

        case TEXTURE_FORMAT_R_F16:
            gl_internal_format = GL_R16F;
            gl_format = GL_RED;
            break;
        case TEXTURE_FORMAT_RG_F16:
            gl_internal_format = GL_RG16F;
            gl_format = GL_RG;
            break;
        case TEXTURE_FORMAT_RGB_F16:
            gl_internal_format = GL_RGB16F;
            gl_format = GL_RGB;
            break;
        case TEXTURE_FORMAT_RGBA_F16:
            gl_internal_format = GL_RGBA16F;
            gl_format = GL_RGBA;
            break;

        case TEXTURE_FORMAT_R_F32:
            gl_internal_format = GL_R32F;
            gl_format = GL_RED;
            break;
        case TEXTURE_FORMAT_RG_F32:
            gl_internal_format = GL_RG32F;
            gl_format = GL_RG;
            break;
        case TEXTURE_FORMAT_RGB_F32:
            gl_internal_format = GL_RGB32F;
            gl_format = GL_RGB;
            break;
        case TEXTURE_FORMAT_RGBA_F32:
            gl_internal_format = GL_RGBA32F;
            gl_format = GL_RGBA;
            break;
    }

    u32 gl_type;
    switch (desc.format) {
        case TEXTURE_FORMAT_R_U8:
        case TEXTURE_FORMAT_RG_U8:
        case TEXTURE_FORMAT_RGB_U8:
        case TEXTURE_FORMAT_RGBA_U8:
            gl_type = GL_UNSIGNED_BYTE;
            break;

        case TEXTURE_FORMAT_R_F16:
        case TEXTURE_FORMAT_RG_F16:
        case TEXTURE_FORMAT_RGB_F16:
        case TEXTURE_FORMAT_RGBA_F16:
        case TEXTURE_FORMAT_R_F32:
        case TEXTURE_FORMAT_RG_F32:
        case TEXTURE_FORMAT_RGB_F32:
        case TEXTURE_FORMAT_RGBA_F32:
            gl_type = GL_FLOAT;
            break;
    }

    u32 gl_sampler;
    switch (desc.sampler) {
        case TEXTURE_SAMPLER_LINEAR:
            gl_sampler = GL_LINEAR;
            break;
        case TEXTURE_SAMPLER_NEAREST:
            gl_sampler = GL_NEAREST;
            break;
    }

    texture_t tex = {0};
    glGenTextures(1, &tex.handle);
    glBindTexture(GL_TEXTURE_2D, tex.handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_sampler);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_sampler);

    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            gl_internal_format,
            desc.width,
            desc.height,
            0,
            gl_format,
            gl_type,
            desc.data);

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void texture_destroy(texture_t texture) {
    glDeleteTextures(1, &texture.handle);
}

void texture_bind(texture_t texture, u32 slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.handle);
}

// -- Framebuffer --------------------------------------------------------------

framebuffer_t framebuffer_create(void) {
    framebuffer_t fb = {0};
    glGenFramebuffers(1, &fb.handle);
    return fb;
}

void framebuffer_destroy(framebuffer_t fb) {
    glDeleteFramebuffers(1, &fb.handle);
}

void framebuffer_bind(framebuffer_t fb) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb.handle);
}

void framebuffer_unbind(void) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

extern void framebuffer_attach(framebuffer_t fb,
        framebuffer_attachment_t attachment,
        u32 slot,
        texture_t texture)    {
    u32 gl_attachment;
    switch (attachment) {
        case FRAMEBUFFER_ATTACHMENT_COLOR:
            gl_attachment = GL_COLOR_ATTACHMENT0 + slot;
            break;
        case FRAMEBUFFER_ATTACHMENT_DEPTH:
            gl_attachment = GL_DEPTH_ATTACHMENT;
            break;
        case FRAMEBUFFER_ATTACHMENT_STENCIL:
            gl_attachment = GL_STENCIL_ATTACHMENT;
            break;
    }
    framebuffer_bind(fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER,
            gl_attachment,
            GL_TEXTURE_2D,
            texture.handle,
            0);
    framebuffer_unbind();
}

// -- Pipeline -----------------------------------------------------------------

pipeline_t pipeline_create(pipeline_desc_t desc) {
    pipeline_t pipeline = {
        .shader = desc.shader,
        .vertex_layout = desc.vertex_layout,
    };
    glGenVertexArrays(1, &pipeline.vao_handle);
    return pipeline;
}

void pipeline_destroy(pipeline_t pipeline) {
    glDeleteVertexArrays(1, &pipeline.vao_handle);
}

// -- Command buffer -----------------------------------------------------------

cmd_buffer_t cmd_buffer_init(arena_t* arena) {
    cmd_buffer_t buf = {
        .arena = arena,
    };
    return buf;
}

void cmd_buffer_begin(cmd_buffer_t* buffer) {
    // Reset everything in the buffer except for the arena.
    *buffer = (cmd_buffer_t) {
        .arena = buffer->arena,
    };
}

void cmd_buffer_end(cmd_buffer_t* buffer) {
    (void) buffer;
    // NOTE: Is this really needed?
}

void cmd_buffer_submit(cmd_buffer_t* buffer) {
    framebuffer_t target_fb = framebuffer_create();
    framebuffer_bind(target_fb);

    for (cmd_t *cmd = buffer->list.first; cmd != NULL; cmd = cmd->next) {
        printf("CMD: %u\n", cmd->type);
        cmd_data_t data = cmd->data;
        switch (cmd->type) {
            case CMD_TYPE_RENDER_PASS_BEGIN: {
                render_pass_t rp = data.render_pass;
                if (rp.target.handle == 0) {
                    framebuffer_unbind();
                } else {
                    framebuffer_attach(target_fb, FRAMEBUFFER_ATTACHMENT_COLOR, 0, rp.target);
                }
                if (rp.load_op == LOAD_OP_CLEAR) {
                    glClearColor(color_arg(rp.clear_color));
                    glClear(GL_COLOR_BUFFER_BIT);
                }
            } break;
            case CMD_TYPE_RENDER_PASS_END:
                break;

            case CMD_TYPE_BIND_PIPELINE: {
                pipeline_t pipeline = data.pipeline;
                buffer->current_pipeline = pipeline;
                buffer->pipeline_bound = true;
                glBindVertexArray(pipeline.vao_handle);
                shader_use(pipeline.shader);
            } break;
            case CMD_TYPE_BIND_VERTEX_BUFFER: {
                if (!buffer->pipeline_bound) {
                    printf("ERROR: No pipeline bound before binding vertex buffer.\n");
                    return;
                }
                vertex_buffer_t vb = data.vertex_buffer;
                pipeline_t pipeline = buffer->current_pipeline;
                vertex_buffer_bind(vb);
                vertex_layout_t layout = pipeline.vertex_layout;
                for (u32 i = 0; i < layout.attrib_count; i++) {
                    u32 gl_type;
                    switch (layout.attribs[i].type) {
                        case VERTEX_ATTRIB_TYPE_F32:
                            gl_type = GL_FLOAT;
                            break;
                    }
                    glVertexAttribPointer(i,
                            layout.attribs[i].count,
                            gl_type,
                            false,
                            layout.stride,
                            (const void*) (layout.attribs[i].offset * sizeof(u8)));
                    glEnableVertexAttribArray(i);
                }
                buffer->vertex_buffer_bound = true;
            } break;
            case CMD_TYPE_BIND_INDEX_BUFFER: {
                index_buffer_t ib = data.index_buffer;
                buffer->index_buffer_bound = true;
                buffer->current_index_buffer = ib;
                index_buffer_bind(ib);
            } break;

            case CMD_TYPE_DRAW: {
                if (!buffer->pipeline_bound) {
                    printf("ERROR: No pipeline bound before drawing.\n");
                    return;
                }
                if (!buffer->vertex_buffer_bound) {
                    printf("ERROR: No vertex buffer bound before drawing.\n");
                    return;
                }
                glDrawArrays(GL_TRIANGLES, data.draw.count, data.draw.first_offset);
            } break;
            case CMD_TYPE_DRAW_INDEXED: {
                if (!buffer->pipeline_bound) {
                    printf("ERROR: No pipeline bound before drawing indexed.\n");
                    return;
                }
                if (!buffer->vertex_buffer_bound) {
                    printf("ERROR: No vertex buffer bound before drawing indexed.\n");
                    return;
                }
                if (!buffer->index_buffer_bound) {
                    printf("ERROR: No index buffer bound before drawing indexed.\n");
                    return;
                }
                glBindVertexArray(buffer->current_pipeline.vao_handle);
                index_buffer_bind(buffer->current_index_buffer);
                printf("pipeline vao handle: %u\n", buffer->current_pipeline.vao_handle);
                glDrawElements(
                        GL_TRIANGLES,
                        data.draw.count,
                        GL_UNSIGNED_INT,
                        (const void*) (data.draw.first_offset * sizeof(u32)));
            } break;
        }
    }

    framebuffer_destroy(target_fb);
}

#define cmd_buffer_push_cmd(BUFF, TYPE, ...) do { \
    cmd_t* cmd = arena_push_type((BUFF)->arena, cmd_t); \
    *cmd = (cmd_t) { \
        .type = (TYPE), \
        .data = {__VA_ARGS__}, \
    }; \
    if ((BUFF)->list.first == NULL) { \
        (BUFF)->list.first = (BUFF)->list.last = cmd; \
    } else { \
        (BUFF)->list.last->next = cmd; \
        cmd->prev = (BUFF)->list.last; \
        (BUFF)->list.last = cmd; \
    }\
} while(0)

void cmd_render_pass_begin(cmd_buffer_t* buffer, render_pass_t pass) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_RENDER_PASS_BEGIN, .render_pass = pass);
}

void cmd_render_pass_end(cmd_buffer_t* buffer) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_RENDER_PASS_BEGIN);
}

void cmd_bind_pipeline(cmd_buffer_t* buffer, pipeline_t pipeline) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_BIND_PIPELINE, .pipeline = pipeline);
}

void cmd_bind_vertex_buffer(cmd_buffer_t* buffer, vertex_buffer_t vertex_buffer) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_BIND_VERTEX_BUFFER, .vertex_buffer = vertex_buffer);
}

void cmd_bind_index_buffer(cmd_buffer_t* buffer, index_buffer_t index_buffer) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_BIND_INDEX_BUFFER, .index_buffer = index_buffer);
}

void cmd_draw(cmd_buffer_t* buffer, u32 vertex_count, u32 first_vertex) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_DRAW, .draw = { .count = vertex_count, .first_offset = first_vertex });
}

void cmd_draw_indexed(cmd_buffer_t* buffer, u32 index_count, u32 first_index) {
    cmd_buffer_push_cmd(buffer, CMD_TYPE_DRAW_INDEXED, .draw = { .count = index_count, .first_offset = first_index });
}
