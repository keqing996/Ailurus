#version 450

// Full-screen quad vertices
layout(location = 0) out vec2 outUV;

void main() {
    // Generate full-screen triangle
    // Vertices: (-1,-1), (3,-1), (-1,3) in clip space
    outUV = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(outUV * 2.0 - 1.0, 0.0, 1.0);
}
