#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform sampler2D scene;
uniform sampler2D bloom;

vec3 aces_tone_map(const vec3 color)
{
    // https://www.shadertoy.com/view/tdffDl
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d ) + e), 0.0, 1.0);
}

void main() {
    vec3 hdr_color = texture(scene, f_uv).rgb + texture(bloom, f_uv).rgb;

    // Exposure tone mapping
    // const float exposure = 1.0;
    // vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
    vec3 mapped = aces_tone_map(hdr_color);
    // Gamma correction
    const float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0);
}
