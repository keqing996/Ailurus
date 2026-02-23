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
    // 8-tap dual Kawase upsample: 4 diagonal + 4 axis-aligned taps
    vec2 ts = params.texelSize;
    vec2 ht = ts * 0.5;

    vec3 s = vec3(0.0);
    s += texture(coarserLevel, fragUV + vec2(-ht.x, -ht.y)).rgb;
    s += texture(coarserLevel, fragUV + vec2( ht.x, -ht.y)).rgb;
    s += texture(coarserLevel, fragUV + vec2(-ht.x,  ht.y)).rgb;
    s += texture(coarserLevel, fragUV + vec2( ht.x,  ht.y)).rgb;
    s += texture(coarserLevel, fragUV + vec2(-ts.x,  0.0)).rgb * 2.0;
    s += texture(coarserLevel, fragUV + vec2( ts.x,  0.0)).rgb * 2.0;
    s += texture(coarserLevel, fragUV + vec2( 0.0, -ts.y)).rgb * 2.0;
    s += texture(coarserLevel, fragUV + vec2( 0.0,  ts.y)).rgb * 2.0;

    vec3 upsampled = s / 12.0;
    vec3 current = texture(currentLevel, fragUV).rgb;

    outColor = vec4(current + upsampled * params.blendFactor, 1.0);
}
