#include "core.h"
#include "program.h"

#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#endif // __EMSCRIPTEN__

void resize_cb(renderer_t* renderer, i32 width, i32 height) {
    (void) renderer;
    glViewport(0, 0, width, height);
    printf("Resize: %dx%d\n", width, height);
}

void update(renderer_t* rend) {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(100.0f/0xff, 149.0f/0xff, 237.0f/0xff, 1.0f);

    renderer_swap_buffers(rend);
}

i32 main(void) {
    renderer_t* rend = renderer_new(800, 600, "What");
    rend->resize_cb = resize_cb;
    rend->update_cb = update;

    const u8 *version = glGetString(GL_VERSION);
    printf("%s\n", version);

    renderer_run(rend);

    // Cleanup
    renderer_free(rend);

    return 0;
}
