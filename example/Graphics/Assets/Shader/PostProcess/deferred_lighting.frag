#version 450

// Global uniform: camera, lights, shadow maps, IBL (set 0 — same layout as pbr.frag)
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
    vec4 ambientColor;
    vec4 shadowBiasParams;
} globalUniform;

// CSM shadow maps
layout(set = 0, binding = 1) uniform sampler2D shadowMap0;
layout(set = 0, binding = 2) uniform sampler2D shadowMap1;
layout(set = 0, binding = 3) uniform sampler2D shadowMap2;
layout(set = 0, binding = 4) uniform sampler2D shadowMap3;

// IBL textures
layout(set = 0, binding = 5) uniform samplerCube irradianceMap;
layout(set = 0, binding = 6) uniform samplerCube prefilteredMap;
layout(set = 0, binding = 7) uniform sampler2D brdfLUT;

// G-Buffer textures (set 1)
layout(set = 1, binding = 0) uniform sampler2D gBuffer0; // Normal (XYZ) + AO (W)
layout(set = 1, binding = 1) uniform sampler2D gBuffer1; // Albedo (RGB) + Roughness (A)
layout(set = 1, binding = 2) uniform sampler2D gBuffer2; // Metallic (R) + unused (GBA)
layout(set = 1, binding = 3) uniform sampler2D gBufferDepth; // Depth buffer

// Push constant: inverse view-projection for world position reconstruction
layout(push_constant) uniform PushConstants {
    mat4 inverseViewProj;
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// Reconstruct world-space position from depth and UV
vec3 worldPosFromDepth(vec2 uv, float depth) {
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 worldPos = pc.inverseViewProj * clipPos;
    return worldPos.xyz / worldPos.w;
}

// Normal Distribution Function (GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return a2 / denom;
}

// Geometry (Smith-GGX)
float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

// Fresnel (Schlick)
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Cook-Torrance BRDF lighting contribution
vec3 calculateLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, float lightIntensity,
                    vec3 albedo, float metallic, float roughness, vec3 F0) {
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);

    vec3 numerator   = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular    = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * lightColor * lightIntensity * NdotL;
}

// CSM cascade selection
int selectCascade(float viewDepth) {
    for (int i = 0; i < 3; i++) {
        if (viewDepth < globalUniform.cascadeSplitDistances[i])
            return i;
    }
    return 3;
}

// PCF shadow sampling
float sampleShadowMap(sampler2D shadowMap, vec2 uv, float compareDepth, float bias) {
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        return 1.0;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            float depth = texture(shadowMap, uv + vec2(x, y) * texelSize).r;
            shadow += (compareDepth - bias > depth) ? 0.0 : 1.0;
        }
    }
    return shadow / 9.0;
}

float calculateShadow(vec3 worldPos, vec3 viewPos, vec3 N, vec3 L) {
    float viewDepth = length(viewPos);
    int cascadeIndex = selectCascade(viewDepth);

    float normalOffsetDist = globalUniform.shadowBiasParams.z;
    vec3 offsetPos = worldPos + N * normalOffsetDist;

    vec4 lightSpacePos = globalUniform.cascadeViewProjMatrices[cascadeIndex] * vec4(offsetPos, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    projCoords = projCoords * 0.5 + 0.5;
    float currentDepth = projCoords.z;

    float cosTheta = max(dot(N, L), 0.0);
    float bias = globalUniform.shadowBiasParams.x + globalUniform.shadowBiasParams.y * (1.0 - cosTheta);

    if (cascadeIndex == 0)      return sampleShadowMap(shadowMap0, projCoords.xy, currentDepth, bias);
    else if (cascadeIndex == 1) return sampleShadowMap(shadowMap1, projCoords.xy, currentDepth, bias);
    else if (cascadeIndex == 2) return sampleShadowMap(shadowMap2, projCoords.xy, currentDepth, bias);
    else                        return sampleShadowMap(shadowMap3, projCoords.xy, currentDepth, bias);
}

void main() {
    // Read depth — skip sky / empty pixels
    float depth = texture(gBufferDepth, fragUV).r;
    if (depth >= 1.0) {
        outColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }

    // Read G-Buffer
    vec4 gb0 = texture(gBuffer0, fragUV);
    vec4 gb1 = texture(gBuffer1, fragUV);
    vec4 gb2 = texture(gBuffer2, fragUV);

    vec3  N         = normalize(gb0.xyz);
    float ao        = gb0.w;
    vec3  albedo    = gb1.xyz;
    float roughness = gb1.w;
    float metallic  = gb2.x;

    // Reconstruct world-space position
    vec3 worldPos = worldPosFromDepth(fragUV, depth);
    vec3 V = normalize(globalUniform.cameraPosition - worldPos);

    // F0 (base reflectivity)
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);

    // Shadow factor (based on first directional light)
    vec3 viewPos = worldPos - globalUniform.cameraPosition;
    vec3 shadowL = (globalUniform.numDirectionalLights > 0)
        ? normalize(-globalUniform.dirLightDirections[0].xyz)
        : vec3(0.0, 1.0, 0.0);
    float shadow = calculateShadow(worldPos, viewPos, N, shadowL);

    // Directional lights (with shadows)
    for (int i = 0; i < globalUniform.numDirectionalLights; i++) {
        vec3  L         = normalize(-globalUniform.dirLightDirections[i].xyz);
        vec3  lightColor = globalUniform.dirLightColors[i].rgb;
        float intensity  = globalUniform.dirLightColors[i].w;
        Lo += calculateLight(N, V, L, lightColor, intensity, albedo, metallic, roughness, F0) * shadow;
    }

    // Point lights
    for (int i = 0; i < globalUniform.numPointLights; i++) {
        vec3  lightPos = globalUniform.pointLightPositions[i].xyz;
        vec3  L        = normalize(lightPos - worldPos);
        float dist     = length(lightPos - worldPos);
        float c = globalUniform.pointLightAttenuations[i].x;
        float l = globalUniform.pointLightAttenuations[i].y;
        float q = globalUniform.pointLightAttenuations[i].z;
        float attenuation = 1.0 / (c + l * dist + q * dist * dist);
        vec3  lightColor  = globalUniform.pointLightColors[i].rgb;
        float intensity   = globalUniform.pointLightColors[i].w;
        Lo += calculateLight(N, V, L, lightColor, intensity * attenuation, albedo, metallic, roughness, F0);
    }

    // Spot lights
    for (int i = 0; i < globalUniform.numSpotLights; i++) {
        vec3  lightPos  = globalUniform.spotLightPositions[i].xyz;
        vec3  L         = normalize(lightPos - worldPos);
        float dist      = length(lightPos - worldPos);
        vec3  spotDir   = normalize(globalUniform.spotLightDirections[i].xyz);
        float theta     = dot(L, -spotDir);
        float inner     = globalUniform.spotLightCutoffs[i].x;
        float outer     = globalUniform.spotLightCutoffs[i].y;
        float spotIntensity = clamp((theta - outer) / (inner - outer), 0.0, 1.0);
        float c = globalUniform.spotLightAttenuations[i].x;
        float ll = globalUniform.spotLightAttenuations[i].y;
        float q = globalUniform.spotLightAttenuations[i].z;
        float attenuation = 1.0 / (c + ll * dist + q * dist * dist);
        vec3  lightColor  = globalUniform.spotLightColors[i].rgb;
        float intensity   = globalUniform.spotLightColors[i].w;
        Lo += calculateLight(N, V, L, lightColor, intensity * attenuation * spotIntensity, albedo, metallic, roughness, F0);
    }

    // IBL ambient lighting
    float NdotV = max(dot(N, V), 0.0);
    vec3 F_ibl = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD_ibl = (vec3(1.0) - F_ibl) * (1.0 - metallic);

    vec3 irradiance     = texture(irradianceMap, N).rgb;
    vec3 diffuseIBL     = kD_ibl * irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 R               = reflect(-V, N);
    vec3 prefilteredColor = textureLod(prefilteredMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf            = texture(brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specularIBL     = prefilteredColor * (F_ibl * brdf.x + brdf.y);

    vec3 ambient = (diffuseIBL + specularIBL) * ao;
    ambient *= globalUniform.ambientColor.rgb * globalUniform.ambientColor.w;

    vec3 color = ambient + Lo;
    outColor = vec4(color, 1.0);
}
