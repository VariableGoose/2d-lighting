#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform vec4 color;
uniform sampler2D tex;

void main() {
    frag_color = texture(tex, f_uv) * color;
}
