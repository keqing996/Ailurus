#version 450

// layout (binding = 0) uniform MaterialProperty
// {
//     vec3 u_CameraPosition;
//     vec3 u_SelfColor;
//     float u_SpecularShininess;
// };

// layout (location = 0) uniform sampler2D mainTexture;

layout (location = 0) in vec2 v_TexCoord;
layout (location = 1) in vec3 v_Normal;

layout (location = 0) out vec4 color;

void main()
{
    // color = vec4(u_SelfColor, 1) * texture(mainTexture, v_TexCoord);
    color = vec4(v_TexCoord, 1, 1);
}
