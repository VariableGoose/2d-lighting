//
// Web implementation of the program header file.
//

#ifdef __EMSCRIPTEN__

#include "program.h"
#include "core.h"

#include <stdio.h>

#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

#include <EGL/egl.h>
#include <glad/gles2.h>

typedef struct em_renderer_t em_renderer_t;
struct em_renderer_t {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
};

static bool internal_resize_cb(int eventType, const EmscriptenUiEvent *uiEvent __attribute__((nonnull)), void *userData) {
    (void) eventType;
    renderer_t* rend = userData;
    if (rend->resize_cb != NULL) {
        rend->resize_cb(rend, uiEvent->windowInnerWidth, uiEvent->windowInnerHeight);
    }
    return true;
}

renderer_t* renderer_new(u32 width, u32 height, const char *title) {
    em_renderer_t* em_rend = malloc(sizeof(em_renderer_t));
    em_rend->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (em_rend->display == EGL_NO_DISPLAY) {
        printf("ERROR: eglGetDisplay\n");
    }

    i32 major;
    i32 minor;
    if (!eglInitialize(em_rend->display, &major, &minor)) {
        printf("ERROR: eglInitialize\n");
    }
    printf("EGL version: %d.%d\n", major, minor);

    i32 num_configs;
    if (!eglGetConfigs(em_rend->display, NULL, 0, &num_configs)) {
        printf("ERROR: glGetConfigs\n");
    }
    EGLConfig config;
    if (!eglChooseConfig(em_rend->display, (i32[]) {
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

    em_rend->surface = eglCreateWindowSurface(em_rend->display, config, 0, NULL);
    if (em_rend->surface == EGL_NO_SURFACE) {
        printf("ERROR: eglCreateWindowSurface\n");
    }

    em_rend->context = eglCreateContext(em_rend->display, config, EGL_NO_CONTEXT, (i32[]) {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE,
            EGL_NONE
        });
    if (em_rend->context == EGL_NO_CONTEXT) {
        printf("ERROR: eglCreateContext\n");
    }

    if (!eglMakeCurrent(em_rend->display, em_rend->surface, em_rend->surface, em_rend->context)) {
        printf("ERROR: eglMakeCurrent\n");
    }

    renderer_t* rend = malloc(sizeof(renderer_t));
    *rend = (renderer_t) {
        .data = em_rend,
    };

    gladLoaderLoadGLES2();

    emscripten_set_resize_callback("#canvas", NULL, true, internal_resize_cb);
    emscripten_set_canvas_element_size("#canvas", width, height);
    emscripten_set_window_title(title);

    return rend;
}

void renderer_free(renderer_t* renderer) {
    em_renderer_t* em_rend = renderer->data;

    gladLoaderUnloadGLES2();

    eglMakeCurrent(em_rend->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(em_rend->display, em_rend->context);
    eglDestroySurface(em_rend->display, em_rend->surface);
    eglTerminate(em_rend->display);

    free(em_rend);
    renderer->data = NULL;
}

void renderer_swap_buffers(renderer_t* renderer) {
    em_renderer_t* em_rend = renderer->data;
    eglSwapBuffers(em_rend->display, em_rend->surface);
}

static void internal_main_loop(void* user_ptr) {
    renderer_t* renderer = user_ptr;
    if (renderer->update_cb != NULL) {
        renderer->update_cb(renderer);
    }
}

void renderer_run(renderer_t* renderer) {
    i32 w, h;
    emscripten_get_canvas_element_size("#canvas", &w, &h);
    if (renderer->resize_cb != NULL) {
        renderer->resize_cb(renderer, w, h);
    }
    emscripten_set_main_loop_arg(internal_main_loop, renderer, 0, true);
}

#endif // __EMSCRIPTEN__
