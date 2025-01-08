#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

out vec4 frag_color;

in vec2 f_uv;

uniform sampler2D scene;
uniform sampler2D bloom;

void main() {
    vec3 hdr_color = texture(scene, f_uv).rgb + texture(bloom, f_uv).rgb;

    // Exposure tone mapping
    const float exposure = 1.0;
    vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
    // Gamma correction
    const float gamma = 2.2;
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0);
}
