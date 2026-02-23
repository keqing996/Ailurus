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
    // 13-tap downsample filter (CoD: Advanced Warfare / Jimenez 2014)
    vec2 uv = fragUV;
    vec2 ts = params.texelSize;

    vec3 a = texture(inputTexture, uv + ts * vec2(-2.0, -2.0)).rgb;
    vec3 b = texture(inputTexture, uv + ts * vec2( 0.0, -2.0)).rgb;
    vec3 c = texture(inputTexture, uv + ts * vec2( 2.0, -2.0)).rgb;
    vec3 d = texture(inputTexture, uv + ts * vec2(-1.0, -1.0)).rgb;
    vec3 e = texture(inputTexture, uv + ts * vec2( 1.0, -1.0)).rgb;
    vec3 f = texture(inputTexture, uv + ts * vec2(-2.0,  0.0)).rgb;
    vec3 g = texture(inputTexture, uv).rgb;
    vec3 h = texture(inputTexture, uv + ts * vec2( 2.0,  0.0)).rgb;
    vec3 i = texture(inputTexture, uv + ts * vec2(-1.0,  1.0)).rgb;
    vec3 j = texture(inputTexture, uv + ts * vec2( 1.0,  1.0)).rgb;
    vec3 k = texture(inputTexture, uv + ts * vec2(-2.0,  2.0)).rgb;
    vec3 l = texture(inputTexture, uv + ts * vec2( 0.0,  2.0)).rgb;
    vec3 m = texture(inputTexture, uv + ts * vec2( 2.0,  2.0)).rgb;

    // Weighted sum: center 4-tap box (weight 0.5) + 4 outer 2x2 boxes (weight 0.125 each)
    vec3 result = (d + e + i + j) * 0.125;
    result += (a + b + f + g) * 0.03125;   // top-left box
    result += (b + c + g + h) * 0.03125;   // top-right box
    result += (f + g + k + l) * 0.03125;   // bottom-left box
    result += (g + h + l + m) * 0.03125;   // bottom-right box

    outColor = vec4(result, 1.0);
}
