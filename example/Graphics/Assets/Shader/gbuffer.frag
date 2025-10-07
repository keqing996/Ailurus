#version 450

layout(set = 1, binding = 0) uniform MaterialProperty {
    vec3 u_SelfColor;
} materialProperty;

layout(set = 1, binding = 1) uniform sampler2D mainTexture;

layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inWorldNormal;
layout(location = 2) in vec2 inUV;

// G-Buffer outputs
layout(location = 0) out vec4 outPosition;   // World space position
layout(location = 1) out vec4 outNormal;     // World space normal
layout(location = 2) out vec4 outAlbedo;     // Base color
layout(location = 3) out vec4 outMaterial;   // Material properties (metallic, roughness, ao)

void main() {
    // Output world position
    outPosition = vec4(inWorldPos, 1.0);
    
    // Output normalized world normal
    outNormal = vec4(normalize(inWorldNormal), 1.0);
    
    // Output albedo (combine texture and material color)
    vec4 texColor = texture(mainTexture, inUV);
    outAlbedo = texColor * vec4(materialProperty.u_SelfColor, 1.0);
    
    // Output material properties
    // R: metallic (0.0 for non-metallic)
    // G: roughness (0.5 default)
    // B: ambient occlusion (1.0 - no occlusion)
    // A: unused
    outMaterial = vec4(0.0, 0.5, 1.0, 1.0);
}
