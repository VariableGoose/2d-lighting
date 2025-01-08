#version 300 es

layout (location = 0) in vec3 v_pos;
layout (location = 1) in vec2 v_uv;

out vec2 f_uv;

uniform mat4 proj;
uniform mat4 transform;

void main() {
    f_uv = v_uv;

    gl_Position = proj * transform * vec4(v_pos, 1.0);
}
