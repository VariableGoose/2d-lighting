#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform vec4 color;
uniform float intensity;

void main() {
    vec2 center = vec2(0.5);
    float length = length(center - f_uv) * 2.0f;
    frag_color = vec4(color.rgb, (1.0 - length) * intensity);
}
