#include "core.h"
#include "program.h"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <glad/gles2.h>
#else
#include <glad/gl.h>
#endif // __EMSCRIPTEN__

void update(renderer_t* rend) {
    app_update(rend->user_ptr);
    renderer_swap_buffers(rend);
}

void resize_cb(renderer_t* renderer, i32 width, i32 height) {
    (void) renderer;
    printf("Resize: %dx%d\n", width, height);
    glViewport(0, 0, width, height);
}

i32 main(void) {
    renderer_t* rend = renderer_new(800, 600, "Cross-platform rendering");
    rend->resize_cb = resize_cb;
    rend->update_cb = update;

    app_t* app = app_init();
    rend->user_ptr = app;

    const u8 *version = glGetString(GL_VERSION);
    printf("%s\n", version);

    renderer_run(rend);

    // Cleanup
    app_shutdown(app);
    renderer_free(rend);

    return 0;
}
