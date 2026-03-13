#version 450

layout(set = 0, binding = 0) uniform sampler2D sceneColor;
layout(set = 0, binding = 1) uniform sampler2D ssaoTexture;

layout(push_constant) uniform PushConstants {
    float ssaoStrength;
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = texture(sceneColor, fragUV).rgb;
    float ao = texture(ssaoTexture, fragUV).r;

    // Blend between full color and AO-modulated color based on strength
    ao = mix(1.0, ao, pc.ssaoStrength);

    outColor = vec4(color * ao, 1.0);
}
