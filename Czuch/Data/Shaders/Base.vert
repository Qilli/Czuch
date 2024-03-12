#version 450

layout(location=0)in vec3 inPosition;
layout(location=1)in vec4 inColor;
layout(location=2)in vec2 inUV;
layout(location=0)out vec4 outColor;
layout(location=1)out vec2 outUV;

layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
} sceneData;

layout( push_constant ) uniform constants
{
 mat4 worldViewProj;
} PushConstants;

void main()
{
	gl_Position = PushConstants.worldViewProj*vec4(inPosition.xyz,1.0);
	outColor=inColor;
	outUV = inUV;
}