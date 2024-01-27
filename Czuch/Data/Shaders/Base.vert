#version 450

layout(location=0)in vec2 inPosition;
layout(location=1)in vec3 inColor;
layout(location=0)out vec3 outColor;

layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
} sceneData;

void main()
{
	gl_Position = vec4(inPosition,0.0,1.0);
	outColor=sceneData.ambientColor.xyz;
}