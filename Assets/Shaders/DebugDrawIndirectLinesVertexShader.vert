#version 460
#extension GL_GOOGLE_include_directive : require
#include "CommonInput.glsl"

struct LineInstanceData
{
	vec4 start;
    vec4 end;
    vec4 color;
};

layout(location=0) in vec3 inPosition;

layout(std430, set = 1, binding = 0) readonly buffer LineInstances {
    LineInstanceData lines[];
};

layout(location=0) out vec4 outColor;

void main() {
     outColor = lines[gl_InstanceIndex].color;
    vec4 worldPos = mix(lines[gl_InstanceIndex].start, lines[gl_InstanceIndex].end, inPosition.x);
    gl_Position =sceneData.viewproj * worldPos;
}