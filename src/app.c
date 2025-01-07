#include "core.h"
#include "program.h"
#include "render_api.h"

app_t* app_init(void) {
    arena_t* arena = arena_new(1<<30);
    app_t* app = arena_push_type(arena, app_t);

    *app = (app_t) {
        .arena = arena,
        .final_pass = render_pass_create((render_pass_desc_t) {
                // Target the swapchain
                .target = {0},
                .load_op = LOAD_OP_CLEAR,
                .clear_color = color_rgb_hex(0x212121),
            }),
    };

    return app;
}

void app_shutdown(app_t* app) {
    arena_free(app->arena);
}

void app_update(app_t* app) {
    RENDER_PASS(&app->final_pass) {
    }
}
