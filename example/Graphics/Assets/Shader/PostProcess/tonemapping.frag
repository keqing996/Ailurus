#version 450

layout(set = 0, binding = 0) uniform sampler2D inputTexture;

layout(push_constant) uniform Params
{
    float exposure;
    float gamma;
} params;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 hdr = texture(inputTexture, fragUV).rgb;
    hdr *= params.exposure;

    // ACES tone mapping (Stephen Hill fit approximation)
    // Coefficients: a=2.51, b=0.03, c=2.43, d=0.59, e=0.14
    vec3 mapped = (hdr * (2.51 * hdr + 0.03)) / (hdr * (2.43 * hdr + 0.59) + 0.14);
    mapped = clamp(mapped, 0.0, 1.0);

    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / params.gamma));

    outColor = vec4(mapped, 1.0);
}
