#version 450

layout(push_constant) uniform PushConstants {
    mat4 inverseViewProjection;
} pushConstants;

layout(location = 0) out vec3 fragWorldDir;

void main()
{
    // Fullscreen triangle using gl_VertexIndex
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    vec4 clipPos = vec4(uv * 2.0 - 1.0, 1.0, 1.0);

    // Transform clip-space position to world-space direction
    vec4 worldPos = pushConstants.inverseViewProjection * clipPos;
    fragWorldDir = worldPos.xyz / worldPos.w;

    // Output at far plane depth (z=1.0, w=1.0)
    gl_Position = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
}
