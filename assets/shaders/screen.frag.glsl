#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform sampler2D obj;
uniform sampler2D light;

void main() {
    vec3 obj_color = texture(obj, f_uv).rgb;
    vec3 light_color = texture(light, f_uv).rgb;
    frag_color = vec4(obj_color * light_color, 1.0);
}
