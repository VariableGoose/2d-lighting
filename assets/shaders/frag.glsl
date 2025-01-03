#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout (location = 0) out vec4 frag_color;

in vec4 f_color;

void main() {
    frag_color = f_color;
}
