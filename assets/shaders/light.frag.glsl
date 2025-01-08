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
    float len = length(center - f_uv) * 2.0f;
    len = clamp(len, 0.0, 1.0);

    float attenuation = smoothstep(1.0, 0.0, len) * intensity;

    vec3 norm_color = color.rgb / max(length(color.rgb), 0.001);
    vec3 light_color = norm_color * attenuation;

    frag_color = vec4(light_color, 1.0);
}
