#version 450

layout(location=0)in vec3 inPosition;
layout(location=1)in vec3 inColor;
layout(location=2)in vec2 inUV;
layout(location=0)out vec3 outColor;
layout(location=1)out vec2 outUV;

layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
} sceneData;

void main()
{
	gl_Position = vec4(inPosition.xyz,1.0);
	outColor=sceneData.ambientColor.xyz;
	outUV = inUV;
}