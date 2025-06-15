#version 460
#extension GL_GOOGLE_include_directive : require
#include "CommonInput.glsl"

struct PointInstanceData
{
	vec4 positionWithSize;
    vec4 color;
};

layout(location=0) in vec3 inPosition;

layout(std430, set = 1, binding = 0) readonly buffer PointInstances {
    PointInstanceData points[];
};

layout(location=0) out vec4 outColor;

void main() {
    outColor = points[gl_InstanceIndex].color;
    vec4 worldPos = points[gl_InstanceIndex].positionWithSize;
    gl_PointSize = worldPos.w; // Use the w component for point size
    worldPos.w=1.0f;
    gl_Position =sceneData.viewproj * worldPos;
}