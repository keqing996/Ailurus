#version 450

layout(set = 0, binding = 0) uniform sampler2D inputTexture;

layout(push_constant) uniform Params
{
    vec2 texelSize;
} params;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    // Dual Kawase downsample: center weighted + 4 half-pixel diagonal taps
    vec2 ht = params.texelSize * 0.5;
    vec3 center = texture(inputTexture, fragUV).rgb * 4.0;
    vec3 tl = texture(inputTexture, fragUV + vec2(-ht.x, -ht.y)).rgb;
    vec3 tr = texture(inputTexture, fragUV + vec2( ht.x, -ht.y)).rgb;
    vec3 bl = texture(inputTexture, fragUV + vec2(-ht.x,  ht.y)).rgb;
    vec3 br = texture(inputTexture, fragUV + vec2( ht.x,  ht.y)).rgb;

    outColor = vec4((center + tl + tr + bl + br) / 8.0, 1.0);
}
