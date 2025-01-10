#version 460

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_color;

layout(location = 0) out vec4 frag_color;

layout(push_constant) uniform PushConstants {
    mat4 render_matrix;
    float time;
} constants;

void main() {
    gl_Position = constants.render_matrix * vec4(in_position, 1.0f);
    frag_color = vec4(in_color, 1.0f) + vec4(vec3(abs(sin(constants.time) / 4.0f)), 0.0f);
}
