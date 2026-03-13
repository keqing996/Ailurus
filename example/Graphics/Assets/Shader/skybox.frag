#version 450

layout(set = 0, binding = 0) uniform samplerCube skyboxTexture;

layout(location = 0) in vec3 fragWorldDir;
layout(location = 0) out vec4 outColor;

void main()
{
    vec3 dir = normalize(fragWorldDir);
    outColor = texture(skyboxTexture, dir);
}
