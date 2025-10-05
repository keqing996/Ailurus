#version 450

// layout (binding = 0) uniform MaterialProperty
// {
//     vec3 u_CameraPosition;
//     vec3 u_SelfColor;
//     float u_SpecularShininess;
// };

layout(set = 1, binding = 1) uniform sampler2D mainTexture;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform MaterialProperty {
    vec3 u_SelfColor;
} materialProperty;

void main() {
    outColor = texture(mainTexture, inUV);
}