#include <glad/gl.h>
#include "core.h"
#include "program.h"
#include "render_api.h"
#include <stdio.h>

typedef struct vert_t vert_t;
struct vert_t {
    Vec2 pos;
    Vec2 uv;
};

typedef struct light_t light_t;
struct light_t {
    Vec3 pos;
    Vec3 size;
    float intensity;
    color_t color;
};

static Quad quad_init(void) {
    vert_t verts[] = {
        { vec2(-0.5f, -0.5f), vec2(0.0f, 0.0f) },
        { vec2( 0.5f, -0.5f), vec2(1.0f, 0.0f) },
        { vec2(-0.5f,  0.5f), vec2(0.0f, 1.0f) },
        { vec2( 0.5f,  0.5f), vec2(1.0f, 1.0f) },
    };
    vertex_buffer_t vb = vertex_buffer_create(verts, sizeof(verts), BUFFER_USAGE_STATIC);

    u32 indices[] = {
        0, 1, 2,
        2, 3, 1,
    };
    index_buffer_t ib = index_buffer_create(indices, arr_len(indices), BUFFER_USAGE_STATIC);

    pipeline_t pipeline = pipeline_create((pipeline_desc_t) {
            .blend = {
                .enabled = true,
                .color_op = BLEND_OP_ADD,
                .src_color_factor = BLEND_FACTOR_SRC_ALPHA,
                .dst_color_factor = BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .alpha_op = BLEND_OP_ADD,
                .src_alpha_factor = BLEND_FACTOR_ONE,
                .dst_alpha_factor = BLEND_FACTOR_ZERO,
            },
            .vertex_buffer = vb,
            .vertex_layout = {
                .stride = sizeof(vert_t),
                .attribs = (vertex_attribute_t[]) {
                    [0] = {
                        .type = VERTEX_ATTRIB_TYPE_F32,
                        .count = 2,
                        .offset = offset(vert_t, pos),
                    },
                    [1] = {
                        .type = VERTEX_ATTRIB_TYPE_F32,
                        .count = 2,
                        .offset = offset(vert_t, uv),
                    },
                },
                .attrib_count = 2,
            },
        });

    return (Quad) {
        .vb = vb,
        .ib = ib,
        .pipe = pipeline,
    };
}

static void draw_quad(Quad quad) {
    pipeline_bind(quad.pipe);
    index_buffer_bind(quad.ib);
    draw_indexed(quad.ib.count, 0);
}

static post_processing_t post_processing_init(arena_t* arena, str_t vert) {
    str_t color_correction_frag = str_read_file(arena, str_lit("assets/shaders/color_correction.frag.glsl"));
    str_t bloom_downsample_sample_frag = str_read_file(arena, str_lit("assets/shaders/bloom_downsample.frag.glsl"));
    str_t bloom_upsample_sample_frag = str_read_file(arena, str_lit("assets/shaders/bloom_upsample.frag.glsl"));

    u32 texture_count = 0;
    Ivec2 screen_size = ivec2(800, 600);
    while (screen_size.y >= 2) {
        texture_count++;
        screen_size = ivec2_divs(screen_size, 2);
    }
    printf("Needed bloom textures: %d\n", texture_count);

    texture_t* textures = arena_push_array(arena, texture_t, texture_count);
    render_pass_t* passes = arena_push_array(arena, render_pass_t, texture_count);
    screen_size = ivec2(800, 600);
    for (u32 i = 0; i < texture_count; i++) {
        textures[i] = texture_create((texture_desc_t) {
                .sampler = TEXTURE_SAMPLER_LINEAR,
                .format = TEXTURE_FORMAT_RGB_F16,
                .width = screen_size.x,
                .height = screen_size.y,
            });
        passes[i] = render_pass_create((render_pass_desc_t) {
                .targets = {textures[i]},
                .target_count = 1,
                .load_op = LOAD_OP_LOAD,
            });

        screen_size = ivec2_divs(screen_size, 2);
    }


    return (post_processing_t) {
        .pass = render_pass_create((render_pass_desc_t) {
                .target_count = 0,
                .load_op = LOAD_OP_CLEAR,
            }),
        .color_correction = {
            .shader = shader_create(vert, color_correction_frag),
        },
        .bloom = {
            .texture_count = texture_count,
            .textures = textures,
            .passes = passes,
            .shader_downsample = shader_create(vert, bloom_downsample_sample_frag),
            .shader_upsample = shader_create(vert, bloom_upsample_sample_frag),
        },
    };
}

app_t* app_init(void) {
    arena_t* arena = arena_new(1<<20);
    app_t* app = arena_push_type(arena, app_t);

    str_t vert = str_read_file(arena, str_lit("assets/shaders/vert.glsl"));
    str_t obj_frag = str_read_file(arena, str_lit("assets/shaders/obj.frag.glsl"));
    str_t light_frag = str_read_file(arena, str_lit("assets/shaders/light.frag.glsl"));
    str_t screen_frag = str_read_file(arena, str_lit("assets/shaders/screen.frag.glsl"));

    texture_t white_texture = texture_create((texture_desc_t) {
            .data = (u8[]) {255, 255, 255, 255},
            .width = 1,
            .height = 1,
            .format = TEXTURE_FORMAT_RGBA_U8,
            .sampler = TEXTURE_SAMPLER_LINEAR,
        });

    texture_t obj_render_target = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_F16,
            .sampler = TEXTURE_SAMPLER_LINEAR,
        });

    texture_t light_render_target = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_F16,
            .sampler = TEXTURE_SAMPLER_LINEAR,
        });

    texture_t comp_render_target = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_F16,
            .sampler = TEXTURE_SAMPLER_LINEAR,
        });

    texture_t bloom_map_render_target = texture_create((texture_desc_t) {
            .data = NULL,
            .width = 800,
            .height = 600,
            .format = TEXTURE_FORMAT_RGBA_F16,
            .sampler = TEXTURE_SAMPLER_LINEAR,
        });

    *app = (app_t) {
        .arena = arena,

        .quad = quad_init(),
        .obj_shader = shader_create(vert, obj_frag),
        .light_shader = shader_create(vert, light_frag),
        .screen_shader = shader_create(vert, screen_frag),
        .white_texture = white_texture,

        .obj_render_target = obj_render_target,
        .obj_pass = render_pass_create((render_pass_desc_t) {
                .targets = {obj_render_target},
                .target_count = 1,
                .load_op = LOAD_OP_CLEAR,
                .clear_color = COLOR_TRANSPARENT,
            }),

        .light_render_target = light_render_target,
        .light_pass = render_pass_create((render_pass_desc_t) {
                .targets = {light_render_target},
                .target_count = 1,
                .load_op = LOAD_OP_CLEAR,
                .clear_color = COLOR_TRANSPARENT,
            }),

        .comp_render_target = comp_render_target,
        .bloom_map_render_target = bloom_map_render_target,
        .comp_pass = render_pass_create((render_pass_desc_t) {
                .targets = {comp_render_target, bloom_map_render_target},
                .target_count = 2,
                .load_op = LOAD_OP_CLEAR,
                .clear_color = COLOR_BLACK,
            }),

        .pp = post_processing_init(arena, vert),

        .screen_pass = render_pass_create((render_pass_desc_t) {
                // Target the swapchain
                .targets = {0},
                .target_count = 0,
                .load_op = LOAD_OP_CLEAR,
                .clear_color = COLOR_BLACK,
            }),
    };

    return app;
}

void app_shutdown(app_t* app) {
    arena_free(app->arena);
}

void app_update(app_t* app) {
    const f32 aspect = 800.0f / 600.0f;
    const f32 zoom = 5.0f;
    Mat4 proj = mat4_ortho_projection(-aspect*zoom, aspect*zoom, zoom, -zoom, 1.0f, -1.0f);

    // Object pass
    RENDER_PASS(&app->obj_pass) {
        {
            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_translate(transform, vec3(1.0f, 1.0f, 0.0f));
            transform = mat4_scale(transform, vec3(1.0f, 1.0f, 1.0f));

            texture_bind(app->white_texture, 0);
            shader_use(app->obj_shader);
            color_t color = color_rgb_hex(0xff00ff);
            Vec4 v4_color = *(Vec4 *) &color;
            shader_uniform_vec4(app->obj_shader, "color", v4_color);
            shader_uniform_mat4(app->obj_shader, "proj", proj);
            shader_uniform_mat4(app->obj_shader, "transform", transform);

            draw_quad(app->quad);
        }

        {
            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_translate(transform, vec3(0.0f, 0.0f, 0.0f));
            transform = mat4_scale(transform, vec3(1.0f, 1.0f, 1.0f));

            texture_bind(app->white_texture, 0);
            shader_use(app->obj_shader);
            color_t color = color_rgb_hex(0x00ff00);
            Vec4 v4_color = *(Vec4 *) &color;
            shader_uniform_vec4(app->obj_shader, "color", v4_color);
            shader_uniform_mat4(app->obj_shader, "proj", proj);
            shader_uniform_mat4(app->obj_shader, "transform", transform);

            draw_quad(app->quad);
        }

        {
            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_translate(transform, vec3(-1.0f, -1.0f, 0.0f));
            transform = mat4_scale(transform, vec3(1.0f, 1.0f, 1.0f));

            texture_bind(app->white_texture, 0);
            shader_use(app->obj_shader);
            color_t color = color_rgb_f(1.0f, 0.2f, 0.2f);
            Vec4 v4_color = *(Vec4 *) &color;
            shader_uniform_vec4(app->obj_shader, "color", v4_color);
            shader_uniform_mat4(app->obj_shader, "proj", proj);
            shader_uniform_mat4(app->obj_shader, "transform", transform);

            draw_quad(app->quad);
        }
    }

    // Light pass
    f32 circle_radius = 4.0f;
    light_t lights[2] = {
        [0] = {
            .pos = vec3(
                    cosf(get_time() * 2.0f + PI) * circle_radius,
                    sinf(get_time() * 2.0f + PI) * circle_radius,
                    0.0f
                ),
            .size = vec3(circle_radius * 2.0f, circle_radius * 2.0f, 1.0f),
            .color = color_rgb_hex(0x80ff33),
            .intensity = 3.0f,
        },
        [1] = {
            .pos = vec3(
                    cosf(get_time() * 2.0f) * circle_radius,
                    sinf(get_time() * 2.0f) * circle_radius,
                    0.0f
                ),
            .size = vec3(circle_radius * 2.0f, circle_radius * 2.0f, 1.0f),
            .color = color_rgb_hex(0xff8033),
            .intensity = 3.0f,
        },
    };
    RENDER_PASS(&app->light_pass) {
        for (u32 i = 0; i < arr_len(lights); i++) {
            light_t light = lights[i];

            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_translate(transform, light.pos);
            transform = mat4_scale(transform, light.size);

            texture_bind(app->white_texture, 0);
            shader_use(app->light_shader);
            // Vert
            shader_uniform_mat4(app->light_shader, "proj", proj);
            shader_uniform_mat4(app->light_shader, "transform", transform);
            // Frag
            Vec4 v4_color = *(Vec4 *) &light.color;
            shader_uniform_vec4(app->light_shader, "color", v4_color);

            shader_uniform_f32(app->light_shader, "intensity", light.intensity);

            draw_quad(app->quad);
        }
    }

    // Composition pass
    RENDER_PASS(&app->comp_pass) {
        Mat4 transform = MAT4_IDENTITY;
        transform = mat4_scale(transform, vec3(2.0f, 2.0f, 1.0f));

        texture_bind(app->obj_render_target, 0);
        texture_bind(app->light_render_target, 1);
        shader_use(app->screen_shader);
        // Vert
        shader_uniform_mat4(app->screen_shader, "proj", MAT4_IDENTITY);
        shader_uniform_mat4(app->screen_shader, "transform", transform);
        // Frag
        shader_uniform_i32(app->screen_shader, "obj", 0);
        shader_uniform_i32(app->screen_shader, "light", 1);
        shader_uniform_vec4(app->screen_shader, "ambient_color", vec4s(1.0f));

        draw_quad(app->quad);
    }

    //
    // -- Post processing ------------------------------------------------------
    //

    // Bloom
    // https://catlikecoding.com/unity/tutorials/advanced-rendering/bloom/
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Bloom");
    // Downsample
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Downsample");
    post_processing_t pp = app->pp;
    texture_t src_texture = app->bloom_map_render_target;
    for (u32 i = 0; i < pp.bloom.texture_count; i++) {
        glViewport(0, 0, vec2_arg(pp.bloom.textures[i].size));
        RENDER_PASS(&pp.bloom.passes[i]) {
            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_scale(transform, vec3(2.0f, 2.0f, 1.0f));

            texture_bind(src_texture, 0);
            shader_use(pp.bloom.shader_downsample);
            // Vert
            shader_uniform_mat4(pp.bloom.shader_downsample, "proj", MAT4_IDENTITY);
            shader_uniform_mat4(pp.bloom.shader_downsample, "transform", transform);

            draw_quad(app->quad);
            src_texture = pp.bloom.textures[i];
        }
    }
    glPopDebugGroup();
    glViewport(0, 0, 800, 600);

    // Upsample
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Upsample");
    src_texture = pp.bloom.textures[pp.bloom.texture_count - 1];
    for (i32 i = pp.bloom.texture_count - 2; i >= 0; i--) {
        texture_t curr_texture = pp.bloom.textures[i];
        glViewport(0, 0, vec2_arg(curr_texture.size));
        RENDER_PASS(&pp.bloom.passes[i]) {
            Mat4 transform = MAT4_IDENTITY;
            transform = mat4_scale(transform, vec3(2.0f, 2.0f, 1.0f));

            texture_bind(src_texture, 0);
            texture_bind(curr_texture, 1);
            shader_use(pp.bloom.shader_upsample);
            // Vert
            shader_uniform_mat4(pp.bloom.shader_upsample, "proj", MAT4_IDENTITY);
            shader_uniform_mat4(pp.bloom.shader_upsample, "transform", transform);
            // Frag
            shader_uniform_i32(pp.bloom.shader_upsample, "src_texture", 0);
            shader_uniform_i32(pp.bloom.shader_upsample, "curr_texture", 1);

            draw_quad(app->quad);
            src_texture = pp.bloom.textures[i + 1];
        }
    }
    glPopDebugGroup();
    glPopDebugGroup();

    // Color correction pass
    RENDER_PASS(&pp.pass) {
        Mat4 transform = MAT4_IDENTITY;
        transform = mat4_scale(transform, vec3(2.0f, 2.0f, 1.0f));

        texture_bind(app->comp_render_target, 0);
        texture_bind(pp.bloom.textures[0], 1);
        shader_use(pp.color_correction.shader);
        // Vert
        shader_uniform_mat4(pp.color_correction.shader, "proj", MAT4_IDENTITY);
        shader_uniform_mat4(pp.color_correction.shader, "transform", transform);
        // Frag
        shader_uniform_i32(pp.color_correction.shader, "scene", 0);
        shader_uniform_i32(pp.color_correction.shader, "bloom", 1);

        draw_quad(app->quad);
    }
}
