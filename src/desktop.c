//
// Desktop implementation of the program header file.
//

#ifndef __EMSCRIPTEN__

#include "program.h"
#include "core.h"

#include <stdlib.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

// Desktop renderer
typedef struct dt_renderer_t dt_renderer_t;
struct dt_renderer_t {
    GLFWwindow* window;
};

static void internal_resize_cb(GLFWwindow* window, int width, int height) {
    renderer_t* rend = glfwGetWindowUserPointer(window);
    if (rend->resize_cb != NULL) {
        rend->resize_cb(rend, width, height);
    }
}

renderer_t* renderer_new(u32 width, u32 height, const char *title)  {
    renderer_t* rend = malloc(sizeof(renderer_t));

    dt_renderer_t* dt = malloc(sizeof(dt_renderer_t));
    rend->data = dt;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, false);
    dt->window = glfwCreateWindow(width, height, title, NULL, NULL);
    glfwMakeContextCurrent(dt->window);
    glfwSetWindowUserPointer(dt->window, rend);
    // Callbacks
    glfwSetFramebufferSizeCallback(dt->window, internal_resize_cb);

    gladLoadGL(glfwGetProcAddress);

    return rend;
}

void renderer_free(renderer_t* renderer) {
    dt_renderer_t* dt = renderer->data;
    glfwDestroyWindow(dt->window);
    glfwTerminate();

    free(dt);
    free(renderer);
}

void renderer_swap_buffers(renderer_t* renderer) {
    dt_renderer_t* dt = renderer->data;
    glfwSwapBuffers(dt->window);
}

void renderer_run(renderer_t* renderer) {
    dt_renderer_t* dt= renderer->data;
    if (renderer->resize_cb != NULL) {
        i32 w, h;
        glfwGetWindowSize(dt->window, &w, &h);
        renderer->resize_cb(renderer, w, h);
    }
    while (!glfwWindowShouldClose(dt->window)) {
        if (renderer->update_cb != NULL) {
            renderer->update_cb(renderer);
        }
        glfwPollEvents();
    }
}

#endif
