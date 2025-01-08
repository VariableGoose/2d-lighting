#version 300 es

#ifdef GL_ES
precision mediump float;
#endif

layout (location = 0) out vec4 frag_color;

in vec2 f_uv;

uniform sampler2D src_texture;

vec3 samp(sampler2D tex, vec2 uv) {
    return texture(tex, uv).rgb;
}

vec3 box_samp(sampler2D tex, vec2 uv, float delta) {
    vec4 o = (1.0 / vec2(textureSize(tex, 0))).xyxy * vec2(-delta, delta).xxyy;
    vec3 s = samp(tex, uv + o.xy) +
             samp(tex, uv + o.zy) +
             samp(tex, uv + o.xw) +
             samp(tex, uv + o.zw);
    return s * 0.25;
}

void main() {
    vec3 c = box_samp(src_texture, f_uv, 1.0);
    frag_color = vec4(c, 1.0);
}
