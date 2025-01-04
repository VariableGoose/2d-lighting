#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout (location = 0) out vec4 frag_color;

in vec4 f_color;
in vec2 f_uv;

uniform sampler2D tex;

void main() {
    frag_color = texture(tex, f_uv) * f_color;
}
