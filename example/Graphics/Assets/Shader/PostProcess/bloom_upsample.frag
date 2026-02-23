#version 450

layout(set = 0, binding = 0) uniform sampler2D coarserLevel;
layout(set = 0, binding = 1) uniform sampler2D currentLevel;

layout(push_constant) uniform Params
{
    vec2 texelSize;
    float blendFactor;
} params;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    // 9-tap tent filter on the coarser (upsampled) level
    vec2 ts = params.texelSize;
    vec3 a = texture(coarserLevel, fragUV + ts * vec2(-1.0, -1.0)).rgb;
    vec3 b = texture(coarserLevel, fragUV + ts * vec2( 0.0, -1.0)).rgb * 2.0;
    vec3 c = texture(coarserLevel, fragUV + ts * vec2( 1.0, -1.0)).rgb;
    vec3 d = texture(coarserLevel, fragUV + ts * vec2(-1.0,  0.0)).rgb * 2.0;
    vec3 e = texture(coarserLevel, fragUV).rgb * 4.0;
    vec3 f = texture(coarserLevel, fragUV + ts * vec2( 1.0,  0.0)).rgb * 2.0;
    vec3 g = texture(coarserLevel, fragUV + ts * vec2(-1.0,  1.0)).rgb;
    vec3 h = texture(coarserLevel, fragUV + ts * vec2( 0.0,  1.0)).rgb * 2.0;
    vec3 k = texture(coarserLevel, fragUV + ts * vec2( 1.0,  1.0)).rgb;

    vec3 upsampled = (a + b + c + d + e + f + g + h + k) / 16.0;
    vec3 current = texture(currentLevel, fragUV).rgb;

    outColor = vec4(current + upsampled * params.blendFactor, 1.0);
}
