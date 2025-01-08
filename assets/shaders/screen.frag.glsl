#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 bloom_color;

in vec2 f_uv;

uniform sampler2D obj;
uniform sampler2D light;
uniform vec4 ambient_color;

void main() {
    vec4 obj_color_full = texture(obj, f_uv);
    vec3 obj_color = obj_color_full.rgb;
    float obj_alpha = obj_color_full.a;
    vec3 light_color = texture(light, f_uv).rgb;

    vec3 result_color = obj_color * ambient_color.rgb + light_color;

    if (obj_alpha <= 0.0) {
        result_color = obj_color;
    }

    frag_color = vec4(result_color, 1.0);

    vec3 bright = max(result_color - vec3(1.0), vec3(0.0));
    bloom_color = vec4(bright, 1.0);
}
