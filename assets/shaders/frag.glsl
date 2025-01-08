#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform vec4 color;

void main() {
    frag_color = color;
}
