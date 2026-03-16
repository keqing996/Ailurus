#version 450

// Material uniforms (set 1)
layout(set = 1, binding = 0) uniform MaterialProperty {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;
layout(set = 1, binding = 2) uniform sampler2D normalTexture;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in mat3 fragTBN;

// G-Buffer outputs:
//   attachment 0: World Normal (XYZ) + AO (W)
//   attachment 1: Albedo (RGB) + Roughness (A)
//   attachment 2: Metallic (R) + unused (GBA)
layout(location = 0) out vec4 gBuffer0;
layout(location = 1) out vec4 gBuffer1;
layout(location = 2) out vec4 gBuffer2;

void main() {
    // Sample albedo texture (convert from sRGB to linear)
    vec4 albedoSample = texture(albedoTexture, fragUV);
    vec3 albedoLinear = pow(albedoSample.rgb, vec3(2.2)) * material.albedo;

    // Normal mapping
    vec3 normalSample = texture(normalTexture, fragUV).xyz * 2.0 - 1.0;
    vec3 N = normalize(fragTBN * normalSample);

    // Write G-Buffer
    gBuffer0 = vec4(N, material.ao);
    gBuffer1 = vec4(albedoLinear, material.roughness);
    gBuffer2 = vec4(material.metallic, 0.0, 0.0, 1.0);
}
