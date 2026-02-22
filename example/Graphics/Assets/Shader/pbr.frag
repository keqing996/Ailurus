#version 450

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

layout(set = 1, binding = 0) uniform MaterialProperty {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
} material;

layout(set = 1, binding = 1) uniform sampler2D albedoTexture;

// CSM shadow maps - one sampler per cascade
layout(set = 0, binding = 1) uniform sampler2D shadowMap0;
layout(set = 0, binding = 2) uniform sampler2D shadowMap1;
layout(set = 0, binding = 3) uniform sampler2D shadowMap2;
layout(set = 0, binding = 4) uniform sampler2D shadowMap3;

layout(location = 0) in vec3 fragWorldPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Normal Distribution Function (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

// Geometry Function (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

// Geometry Function (Smith's method)
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

// Fresnel (Schlick approximation)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Calculate lighting contribution from a single light
vec3 calculateLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, float lightIntensity,
                    vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    
    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // Metals have no diffuse
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    
    return (kD * albedo / PI + specular) * lightColor * lightIntensity * NdotL;
}

// Select cascade based on view-space depth
int selectCascade(float viewDepth) {
    for (int i = 0; i < 3; i++) {
        if (viewDepth < globalUniform.cascadeSplitDistances[i]) {
            return i;
        }
    }
    return 3; // Last cascade
}

// PCF shadow sampling
float sampleShadowMap(sampler2D shadowMap, vec2 uv, float compareDepth) {
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        return 1.0; // Outside shadow map bounds
    }
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    // 3x3 PCF
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            vec2 offset = vec2(x, y) * texelSize;
            float depth = texture(shadowMap, uv + offset).r;
            shadow += (compareDepth - 0.005 > depth) ? 0.0 : 1.0;
        }
    }
    
    return shadow / 9.0;
}

// Calculate shadow factor using CSM
float calculateShadow(vec3 worldPos, vec3 viewPos) {
    // Calculate view-space depth
    float viewDepth = length(viewPos);
    
    // Select cascade
    int cascadeIndex = selectCascade(viewDepth);
    
    // Transform to light space
    vec4 lightSpacePos = globalUniform.cascadeViewProjMatrices[cascadeIndex] * vec4(worldPos, 1.0);
    
    // Perspective divide
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Get depth from light's perspective
    float currentDepth = projCoords.z;
    
    // Sample appropriate shadow map
    float shadow;
    if (cascadeIndex == 0) {
        shadow = sampleShadowMap(shadowMap0, projCoords.xy, currentDepth);
    } else if (cascadeIndex == 1) {
        shadow = sampleShadowMap(shadowMap1, projCoords.xy, currentDepth);
    } else if (cascadeIndex == 2) {
        shadow = sampleShadowMap(shadowMap2, projCoords.xy, currentDepth);
    } else {
        shadow = sampleShadowMap(shadowMap3, projCoords.xy, currentDepth);
    }
    
    return shadow;
}

void main() {
    vec3 albedo = material.albedo * texture(albedoTexture, fragUV).rgb;
    float metallic = material.metallic;
    float roughness = material.roughness;
    float ao = material.ao;
    
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(globalUniform.cameraPosition - fragWorldPos);
    
    // Calculate base reflectivity (F0)
    vec3 F0 = vec3(0.04); // Dielectric default
    F0 = mix(F0, albedo, metallic);
    
    vec3 Lo = vec3(0.0);
    
    // Calculate shadow factor for directional lights
    vec3 viewPos = fragWorldPos - globalUniform.cameraPosition;
    float shadow = calculateShadow(fragWorldPos, viewPos);
    
    // Directional lights (with shadows)
    for (int i = 0; i < globalUniform.numDirectionalLights; i++) {
        vec3 L = normalize(-globalUniform.dirLightDirections[i].xyz);
        vec3 lightColor = globalUniform.dirLightColors[i].rgb;
        float intensity = globalUniform.dirLightColors[i].w;
        vec3 lighting = calculateLight(N, V, L, lightColor, intensity, albedo, metallic, roughness, F0);
        Lo += lighting * shadow; // Apply shadow to directional lights
    }
    
    // Point lights
    for (int i = 0; i < globalUniform.numPointLights; i++) {
        vec3 lightPos = globalUniform.pointLightPositions[i].xyz;
        vec3 L = normalize(lightPos - fragWorldPos);
        float distance = length(lightPos - fragWorldPos);
        
        // Attenuation
        float constant = globalUniform.pointLightAttenuations[i].x;
        float linear = globalUniform.pointLightAttenuations[i].y;
        float quadratic = globalUniform.pointLightAttenuations[i].z;
        float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
        
        vec3 lightColor = globalUniform.pointLightColors[i].rgb;
        float intensity = globalUniform.pointLightColors[i].w;
        Lo += calculateLight(N, V, L, lightColor, intensity * attenuation, albedo, metallic, roughness, F0);
    }
    
    // Spot lights
    for (int i = 0; i < globalUniform.numSpotLights; i++) {
        vec3 lightPos = globalUniform.spotLightPositions[i].xyz;
        vec3 L = normalize(lightPos - fragWorldPos);
        float distance = length(lightPos - fragWorldPos);
        
        // Spot light cone
        vec3 spotDir = normalize(globalUniform.spotLightDirections[i].xyz);
        float theta = dot(L, -spotDir);
        float innerCutoff = globalUniform.spotLightCutoffs[i].x; // cos(inner angle)
        float outerCutoff = globalUniform.spotLightCutoffs[i].y; // cos(outer angle)
        float epsilon = innerCutoff - outerCutoff;
        float spotIntensity = clamp((theta - outerCutoff) / epsilon, 0.0, 1.0);
        
        // Attenuation
        float constant = globalUniform.spotLightAttenuations[i].x;
        float linear = globalUniform.spotLightAttenuations[i].y;
        float quadratic = globalUniform.spotLightAttenuations[i].z;
        float attenuation = 1.0 / (constant + linear * distance + quadratic * distance * distance);
        
        vec3 lightColor = globalUniform.spotLightColors[i].rgb;
        float intensity = globalUniform.spotLightColors[i].w;
        Lo += calculateLight(N, V, L, lightColor, intensity * attenuation * spotIntensity, albedo, metallic, roughness, F0);
    }
    
    // Ambient lighting
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    outColor = vec4(color, 1.0);
}
