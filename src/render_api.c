#include "render_api.h"
#include "core.h"

#include <stdio.h>
#include <math.h>

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

// -- Color --------------------------------------------------------------------

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

void shader_uniform_vec4(shader_t shader, const char* name, Vec4 value) {
    u32 loc = glGetUniformLocation(shader.handle, name);
    glUniform4fv(loc, 1, &value.x);
}

void shader_uniform_mat4(shader_t shader, const char* name, Mat4 value) {
    u32 loc = glGetUniformLocation(shader.handle, name);
    glUniformMatrix4fv(loc, 1, false, &value.a.x);
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
}

// -- Pipeline -----------------------------------------------------------------

pipeline_t pipeline_create(pipeline_desc_t desc) {
    pipeline_t pipeline = {
        .desc = desc,
    };
    glGenVertexArrays(1, &pipeline.vao_handle);
    glBindVertexArray(pipeline.vao_handle);
    vertex_buffer_bind(desc.vertex_buffer);

    vertex_layout_t layout = desc.vertex_layout;
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

    return pipeline;
}

void pipeline_destroy(pipeline_t pipeline) {
    glDeleteVertexArrays(1, &pipeline.vao_handle);
}

static GLenum blend_op_to_gl(blend_op_t op) {
    switch (op) {
        case BLEND_OP_ADD:
            return GL_FUNC_ADD;
        case BLEND_OP_SUB:
            return GL_FUNC_SUBTRACT;
    }

    return GL_INVALID_ENUM;
}

static GLenum blend_factor_to_gl(blend_factor_t factor) {
    switch (factor) {
        case BLEND_FACTOR_ZERO:
            return GL_ZERO;
        case BLEND_FACTOR_ONE:
            return GL_ONE;
        case BLEND_FACTOR_SRC_ALPHA:
            return GL_SRC_ALPHA;
        case BLEND_FACTOR_DST_ALPHA:
            return GL_DST_ALPHA;
        case BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
    }

    return GL_INVALID_ENUM;
}

void pipeline_bind(pipeline_t pipeline) {
    glBindVertexArray(pipeline.vao_handle);

    blend_state_t blend = pipeline.desc.blend;
    if (blend.enabled) {
        glEnable(GL_BLEND);

        GLenum gl_color_func = blend_op_to_gl(blend.color_op);
        GLenum gl_alpha_func = blend_op_to_gl(blend.alpha_op);
        glBlendEquationSeparate(gl_color_func, gl_alpha_func);

        GLenum gl_src_color_factor = blend_factor_to_gl(blend.src_color_factor);
        GLenum gl_dst_color_factor = blend_factor_to_gl(blend.dst_color_factor);
        GLenum gl_src_alpha_factor = blend_factor_to_gl(blend.src_alpha_factor);
        GLenum gl_dst_alpha_factor = blend_factor_to_gl(blend.dst_alpha_factor);
        glBlendFuncSeparate(gl_src_color_factor, gl_dst_color_factor,
                gl_src_alpha_factor, gl_dst_alpha_factor);
    } else {
        glDisable(GL_BLEND);
    }
}

// -- Render pass -----------------------------------------------------------

render_pass_t render_pass_create(render_pass_desc_t desc) {
    render_pass_t rp = {
        .target = desc.target,
        .clear_color = desc.clear_color,
        .load_op = desc.load_op,
        .target_fb = framebuffer_create(),
    };
    return rp;
}

void render_pass_destroy(render_pass_t pass) {
    framebuffer_destroy(pass.target_fb);
}

void render_pass_begin(render_pass_t* pass) {
    if (pass->target.handle != 0) {
        framebuffer_bind(pass->target_fb);
        framebuffer_attach(pass->target_fb, FRAMEBUFFER_ATTACHMENT_COLOR, 0, pass->target);
    } else {
        framebuffer_unbind();
    }
    if (pass->load_op == LOAD_OP_CLEAR) {
        glClearColor(color_arg(pass->clear_color));
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

void render_pass_end(render_pass_t* pass) {
    (void) pass;
}

void draw(u32 vertex_count, u32 first_vertex) {
    glDrawArrays(GL_TRIANGLES, vertex_count, first_vertex);
}

void draw_indexed(u32 index_count, u32 first_index) {
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, (const void*) (first_index*sizeof(u32)));
}
