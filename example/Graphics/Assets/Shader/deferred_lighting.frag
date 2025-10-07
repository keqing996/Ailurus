#version 450

// G-Buffer inputs
layout(set = 0, binding = 0) uniform sampler2D gBufferPosition;
layout(set = 0, binding = 1) uniform sampler2D gBufferNormal;
layout(set = 0, binding = 2) uniform sampler2D gBufferAlbedo;
layout(set = 0, binding = 3) uniform sampler2D gBufferMaterial;

// Light data structures
#define MAX_POINT_LIGHTS 16
#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_SPOT_LIGHTS 8

struct PointLight {
    vec3 position;
    float range;
    vec3 color;
    float intensity;
    vec3 attenuation; // constant, linear, quadratic
    float padding;
};

struct DirectionalLight {
    vec3 direction;
    float intensity;
    vec3 color;
    float padding;
};

struct SpotLight {
    vec3 position;
    float range;
    vec3 direction;
    float intensity;
    vec3 color;
    float innerConeAngle;
    float outerConeAngle;
    vec3 attenuation; // constant, linear, quadratic
};

layout(std140, set = 1, binding = 0) uniform LightingData {
    vec3 cameraPosition;
    int numPointLights;
    int numDirectionalLights;
    int numSpotLights;
    vec2 padding;
    
    PointLight pointLights[MAX_POINT_LIGHTS];
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    SpotLight spotLights[MAX_SPOT_LIGHTS];
} lightingData;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

// Calculate attenuation for point/spot lights
float calculateAttenuation(vec3 attenuation, float distance) {
    return 1.0 / (attenuation.x + attenuation.y * distance + attenuation.z * distance * distance);
}

// Simple Blinn-Phong lighting model
vec3 calculateBlinnPhong(vec3 normal, vec3 viewDir, vec3 lightDir, vec3 lightColor, float intensity) {
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor * intensity;
    
    // Specular
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * lightColor * intensity * 0.5;
    
    return diffuse + specular;
}

// Process point light
vec3 processPointLight(PointLight light, vec3 worldPos, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - worldPos);
    float distance = length(light.position - worldPos);
    
    // Check if fragment is within light range
    if (distance > light.range)
        return vec3(0.0);
    
    // Calculate attenuation
    float attenuation = calculateAttenuation(light.attenuation, distance);
    
    // Calculate lighting
    vec3 lighting = calculateBlinnPhong(normal, viewDir, lightDir, light.color, light.intensity);
    
    return lighting * attenuation;
}

// Process directional light
vec3 processDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    return calculateBlinnPhong(normal, viewDir, lightDir, light.color, light.intensity);
}

// Process spot light
vec3 processSpotLight(SpotLight light, vec3 worldPos, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - worldPos);
    float distance = length(light.position - worldPos);
    
    // Check if fragment is within light range
    if (distance > light.range)
        return vec3(0.0);
    
    // Calculate spot cone
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = cos(radians(light.innerConeAngle)) - cos(radians(light.outerConeAngle));
    float spotIntensity = clamp((theta - cos(radians(light.outerConeAngle))) / epsilon, 0.0, 1.0);
    
    if (spotIntensity <= 0.0)
        return vec3(0.0);
    
    // Calculate attenuation
    float attenuation = calculateAttenuation(light.attenuation, distance);
    
    // Calculate lighting
    vec3 lighting = calculateBlinnPhong(normal, viewDir, lightDir, light.color, light.intensity);
    
    return lighting * attenuation * spotIntensity;
}

void main() {
    // Sample G-Buffer
    vec3 worldPos = texture(gBufferPosition, inUV).rgb;
    vec3 normal = normalize(texture(gBufferNormal, inUV).rgb);
    vec3 albedo = texture(gBufferAlbedo, inUV).rgb;
    vec4 material = texture(gBufferMaterial, inUV);
    
    // Calculate view direction
    vec3 viewDir = normalize(lightingData.cameraPosition - worldPos);
    
    // Ambient lighting
    vec3 ambient = albedo * 0.1;
    
    // Accumulate lighting from all lights
    vec3 lighting = vec3(0.0);
    
    // Point lights
    for (int i = 0; i < lightingData.numPointLights; ++i) {
        lighting += processPointLight(lightingData.pointLights[i], worldPos, normal, viewDir);
    }
    
    // Directional lights
    for (int i = 0; i < lightingData.numDirectionalLights; ++i) {
        lighting += processDirectionalLight(lightingData.directionalLights[i], normal, viewDir);
    }
    
    // Spot lights
    for (int i = 0; i < lightingData.numSpotLights; ++i) {
        lighting += processSpotLight(lightingData.spotLights[i], worldPos, normal, viewDir);
    }
    
    // Combine lighting with albedo and ambient
    vec3 finalColor = ambient + lighting * albedo;
    
    // Apply material properties (simplified)
    float ao = material.b;
    finalColor *= ao;
    
    outColor = vec4(finalColor, 1.0);
}
