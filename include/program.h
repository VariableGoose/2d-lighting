#ifndef PROGRAM_H
#define PROGRAM_H

#include "core.h"
#include "render_api.h"

typedef struct renderer_t renderer_t;

typedef void (*resize_callback_t)(renderer_t* renderer, i32 width, i32 height);
typedef void (*update_callback_t)(renderer_t* renderer);

struct renderer_t {
    // Resize callback
    resize_callback_t resize_cb;
    // Update callback; will be called once per frame
    update_callback_t update_cb;
    // User data
    void* user_ptr;

    // Internal data
    void* data;
};

extern renderer_t* renderer_new(u32 width, u32 height, const char *title);
extern void renderer_free(renderer_t* renderer);
extern void renderer_swap_buffers(renderer_t* renderer);
extern void renderer_run(renderer_t* renderer);

// -- App ----------------------------------------------------------------------

typedef struct Quad Quad;
struct Quad {
    vertex_buffer_t vb;
    index_buffer_t ib;
    pipeline_t pipe;
};

typedef struct app_t app_t;
struct app_t {
    arena_t* arena;

    Quad quad;
    shader_t obj_shader;

    render_pass_t final_pass;
};

extern app_t* app_init(void);
extern void app_shutdown(app_t* app);
extern void app_update(app_t* app);

#endif // PROGRAM_H
