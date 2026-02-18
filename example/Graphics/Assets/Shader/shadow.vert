#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
    uint cascadeIndex;
} pushConstants;

layout(std140, set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
    int numDirectionalLights;
    int numPointLights;
    int numSpotLights;
    vec4 dirLightDirections[4];
    vec4 dirLightColors[4];
    vec4 pointLightPositions[8];
    vec4 pointLightColors[8];
    vec4 pointLightAttenuations[8];
    vec4 spotLightPositions[4];
    vec4 spotLightDirections[4];
    vec4 spotLightColors[4];
    vec4 spotLightAttenuations[4];
    vec4 spotLightCutoffs[4];
    mat4 cascadeViewProjMatrices[4];
    float cascadeSplitDistances[4];
} globalUniform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

void main() {
    vec4 worldPos = pushConstants.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = globalUniform.cascadeViewProjMatrices[pushConstants.cascadeIndex] * worldPos;
}
