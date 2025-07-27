#version 460
#extension GL_GOOGLE_include_directive : require
#include "CommonInput.glsl"

struct TriangleInstanceData
{
	vec4 positions[3];
    vec4 color;
};

layout(location=0) in vec3 inPosition;

layout(std430, set = 1, binding = 0) readonly buffer LineInstances {
    TriangleInstanceData tris[];
};

layout(location=0) out vec4 outColor;

void main() {
     outColor = tris[gl_InstanceIndex].color;
    vec4 worldPos = tris[gl_InstanceIndex].positions[gl_VertexIndex];
    gl_Position =sceneData.viewproj * worldPos;
}