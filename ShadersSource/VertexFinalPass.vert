#version 450

const vec2 quadVertices[6] = vec2[6](
    vec2(-1.0,  1.0),  // Top-left
    vec2( 1.0, -1.0),  // Bottom-right
    vec2(-1.0, -1.0),  // Bottom-left

    vec2(-1.0,  1.0),  // Top-left
    vec2( 1.0,  1.0),  // Top-right
    vec2( 1.0, -1.0)   // Bottom-right
);

const vec2 texCoords[6] = vec2[6](
    vec2(0.0, 0.0),    // Top-left
    vec2(1.0, 1.0),    // Bottom-right
    vec2(0.0, 1.0),    // Bottom-left

    vec2(0.0, 0.0),    // Top-left
    vec2(1.0, 0.0),    // Top-right
    vec2(1.0, 1.0)     // Bottom-right
);

layout(location = 0) out vec2 fragTexCoord;

void main()
{
    gl_Position = vec4(quadVertices[gl_VertexIndex], 0.0, 1.0);
    fragTexCoord = texCoords[gl_VertexIndex];
}