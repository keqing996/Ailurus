#version 450

layout(set = 0, binding = 0) uniform sampler2D inputTexture;

layout(push_constant) uniform Params
{
    float threshold;
    float softKnee;
} params;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 color = texture(inputTexture, fragUV).rgb;
    float brightness = max(color.r, max(color.g, color.b));

    // Soft knee: smooth transition around threshold
    float knee = params.threshold * params.softKnee;
    float soft = brightness - params.threshold + knee;
    soft = clamp(soft / (2.0 * knee + 0.00001), 0.0, 1.0);
    soft = soft * soft;

    float contribution = max(soft, brightness - params.threshold) / max(brightness, 0.00001);
    contribution = max(contribution, 0.0);

    outColor = vec4(color * contribution, 1.0);
}
