#version 450

layout(push_constant) uniform PushConstants {
    mat4 mvp;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

void main() {
    gl_Position = pushConstants.mvp * vec4(inPosition, 1.0);
}