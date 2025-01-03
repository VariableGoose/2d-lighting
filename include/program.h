#ifndef PROGRAM_H
#define PROGRAM_H

#include "core.h"

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

#endif // PROGRAM_H
