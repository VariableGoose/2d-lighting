#include "core.h"

#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

typedef struct render_context_t render_context_t;
struct render_context_t {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

void loop(void *user_ptr) {
    render_context_t *render = user_ptr;

    i32 w, h;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(100.0f/0xff, 149.0f/0xff, 237.0f/0xff, 1.0f);

    eglSwapBuffers(render->display, render->surface);
}

i32 main(void) {
    render_context_t render = {0};

    // Init
    render.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (render.display == EGL_NO_DISPLAY) {
        printf("ERROR: eglGetDisplay\n");
    }

    i32 major;
    i32 minor;
    if (!eglInitialize(render.display, &major, &minor)) {
        printf("ERROR: eglInitialize\n");
    }
    printf("EGL version: %d.%d\n", major, minor);

    i32 num_configs;
    if (!eglGetConfigs(render.display, NULL, 0, &num_configs)) {
        printf("ERROR: glGetConfigs\n");
    }
    EGLConfig config;
    if (!eglChooseConfig(render.display, (i32[]) {
            EGL_RED_SIZE,       5,
            EGL_GREEN_SIZE,     6,
            EGL_BLUE_SIZE,      5,
            EGL_ALPHA_SIZE,     8,
            EGL_DEPTH_SIZE,     8,
            EGL_STENCIL_SIZE,   8,
            EGL_SAMPLE_BUFFERS, 1,
            EGL_NONE
        }, &config, 1, &num_configs)) {
        printf("ERROR: elgChooseConfig\n");
    }
    printf("Config count: %d\n", num_configs);

    render.surface = eglCreateWindowSurface(render.display, config, 0, NULL);
    if (render.surface == EGL_NO_SURFACE) {
        printf("ERROR: eglCreateWindowSurface\n");
    }

    render.context = eglCreateContext(render.display, config, EGL_NO_CONTEXT, (i32[]) {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE,
            EGL_NONE
        });
    if (render.context == EGL_NO_CONTEXT) {
        printf("ERROR: eglCreateContext\n");
    }

    if (!eglMakeCurrent(render.display, render.surface, render.surface, render.context)) {
        printf("ERROR: eglMakeCurrent\n");
    }

    const u8 *version = glGetString(GL_VERSION);
    printf("%s\n", version);

#ifdef __EMSCRIPTEN__
    emscripten_set_canvas_element_size("#canvas", 800, 600);
    emscripten_set_main_loop_arg(loop, &render, 0, true);
#else
    while (true) {
        loop(&render);
    }
#endif

    // Cleanup
    eglMakeCurrent(render.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(render.display, render.context);
    eglDestroySurface(render.display, render.surface);
    eglTerminate(render.display);

    return 0;
}
