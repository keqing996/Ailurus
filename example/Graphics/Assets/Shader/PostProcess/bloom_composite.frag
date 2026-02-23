#version 450

layout(set = 0, binding = 0) uniform sampler2D bloomTexture;
layout(set = 0, binding = 1) uniform sampler2D originalTexture;

layout(push_constant) uniform Params
{
    float bloomIntensity;
} params;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 bloom = texture(bloomTexture, fragUV).rgb;
    vec3 original = texture(originalTexture, fragUV).rgb;

    outColor = vec4(original + bloom * params.bloomIntensity, 1.0);
}
