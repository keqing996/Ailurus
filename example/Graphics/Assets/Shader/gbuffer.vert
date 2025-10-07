#version 450

layout(push_constant) uniform PushConstants {
    mat4 modelMatrix;
} pushConstants;

layout(std140, set = 0, binding = 0) uniform GlobalUniform {
    mat4 viewProjectionMatrix;
    vec3 cameraPosition;
} globalUniform;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outWorldNormal;
layout(location = 2) out vec2 outUV;

void main() {
    // Transform position to world space
    vec4 worldPos = pushConstants.modelMatrix * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    
    // Transform normal to world space
    mat3 normalMatrix = transpose(inverse(mat3(pushConstants.modelMatrix)));
    outWorldNormal = normalize(normalMatrix * inNormal);
    
    // Pass UV coordinates
    outUV = inUV;
    
    // Calculate clip space position
    gl_Position = globalUniform.viewProjectionMatrix * worldPos;
}
