#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
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
} globalUniform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragWorldPos;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec2 fragUV;

void main() {
    vec4 worldPos = pushConstants.modelMatrix * vec4(inPosition, 1.0);
    fragWorldPos = worldPos.xyz;
    
    // Transform normal to world space (using transpose of inverse of model matrix for non-uniform scaling)
    mat3 normalMatrix = transpose(inverse(mat3(pushConstants.modelMatrix)));
    fragNormal = normalize(normalMatrix * inNormal);
    
    fragUV = inUV;
    
    gl_Position = globalUniform.viewProjectionMatrix * worldPos;
}
