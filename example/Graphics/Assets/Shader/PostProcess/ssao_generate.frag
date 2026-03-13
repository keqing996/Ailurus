#version 450

layout(set = 0, binding = 0) uniform sampler2D depthTexture;

layout(push_constant) uniform PushConstants {
    mat4 projection;
    float radius;
    float bias;
    float power;
    int kernelSize;
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

// Hash-based pseudo-random function
float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Generate a pseudo-random 3D vector in hemisphere
vec3 randomHemisphereDir(vec2 seed, vec3 normal) {
    float phi = hash(seed) * 6.28318530718;
    float cosTheta = hash(seed + vec2(1.0, 0.0));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    vec3 dir = vec3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
    // Ensure hemisphere aligns with normal direction
    return dir * sign(dot(dir, normal));
}

// Reconstruct view-space position from depth and UV
vec3 viewPosFromDepth(vec2 uv, float depth) {
    // NDC coordinates
    vec4 clipPos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    // Inverse projection
    vec4 viewPos = inverse(pc.projection) * clipPos;
    return viewPos.xyz / viewPos.w;
}

void main() {
    float depth = texture(depthTexture, fragUV).r;

    // Skip sky pixels (depth at far plane)
    if (depth >= 1.0) {
        outColor = vec4(1.0);
        return;
    }

    vec3 viewPos = viewPosFromDepth(fragUV, depth);

    // Estimate normal from depth derivatives
    vec2 texelSize = 1.0 / textureSize(depthTexture, 0);
    float depthR = texture(depthTexture, fragUV + vec2(texelSize.x, 0.0)).r;
    float depthU = texture(depthTexture, fragUV + vec2(0.0, texelSize.y)).r;
    vec3 viewPosR = viewPosFromDepth(fragUV + vec2(texelSize.x, 0.0), depthR);
    vec3 viewPosU = viewPosFromDepth(fragUV + vec2(0.0, texelSize.y), depthU);
    vec3 normal = normalize(cross(viewPosR - viewPos, viewPosU - viewPos));

    // SSAO sampling
    float occlusion = 0.0;
    int samples = pc.kernelSize;
    vec2 pixelSeed = fragUV * textureSize(depthTexture, 0);

    for (int i = 0; i < samples; i++) {
        // Generate random sample in hemisphere
        vec2 seed = pixelSeed + vec2(float(i) * 7.31, float(i) * 3.17);
        vec3 sampleDir = randomHemisphereDir(seed, normal);

        // Distribute samples closer to the surface (accelerating distribution)
        float scale = float(i) / float(samples);
        scale = mix(0.1, 1.0, scale * scale);
        vec3 samplePos = viewPos + sampleDir * pc.radius * scale;

        // Project sample to screen space
        vec4 offset = pc.projection * vec4(samplePos, 1.0);
        offset.xyz /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;

        // Sample depth at projected position
        float sampleDepth = texture(depthTexture, offset.xy).r;
        vec3 sampledViewPos = viewPosFromDepth(offset.xy, sampleDepth);

        // Range check and occlusion test
        float rangeCheck = smoothstep(0.0, 1.0, pc.radius / abs(viewPos.z - sampledViewPos.z));
        occlusion += (sampledViewPos.z >= samplePos.z + pc.bias ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(samples));
    occlusion = pow(occlusion, pc.power);

    outColor = vec4(occlusion, occlusion, occlusion, 1.0);
}
