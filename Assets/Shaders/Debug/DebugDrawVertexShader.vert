#version 460
#extension GL_GOOGLE_include_directive : require
#include "CommonInput.glsl"

layout(location=0) in vec3 inPosition;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec4 inColor;
layout(location=3) in vec2 inUV0;
layout(location=4) in vec2 inUV1;
layout(location=5) in vec4 inTangent;

layout(location=0) out vec4 outColor;
layout(location=1) out vec4 outUV;
layout(location=2) out vec3 outNormal;

void main() {
     gl_Position = PushConstants.worldViewProj * vec4(inPosition.xyz, 1.0);

    outColor = inColor;
    outUV = vec4(inUV0.xy,inUV1.xy);
    outNormal =  inNormal;
}