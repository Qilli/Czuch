#version 450
#extension GL_GOOGLE_include_directive : require
#include "CommonInput.glsl"

layout(location=0)in vec3 inPosition;

void main()
{
	gl_Position = PushConstants.worldViewProj*vec4(inPosition.xyz,1.0);
}