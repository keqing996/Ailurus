#version 450

layout(set = 0, binding = 0) uniform samplerCube environmentMap;

layout(push_constant) uniform PushConstants {
    mat4 faceVP;
} pc;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

void main()
{
    // Convert UV to clip space, then to world direction via inverse faceVP
    vec4 clipPos = vec4(fragUV * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldDir = inverse(pc.faceVP) * clipPos;
    vec3 N = normalize(worldDir.xyz / worldDir.w);

    vec3 irradiance = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // Spherical to cartesian (tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // Tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / nrSamples);

    outColor = vec4(irradiance, 1.0);
}
