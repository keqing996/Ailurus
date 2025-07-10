#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} pushConstants;

layout(set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} globalUniform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec2 outUV;

void main() {
    gl_Position = globalUniform.viewProjectionMatrix * pushConstants.modelMatrix * vec4(inPosition, 1.0);
    outUV = uv;
}