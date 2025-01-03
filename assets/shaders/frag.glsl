#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout (location = 0) out vec4 frag_color;

void main() {
    frag_color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
