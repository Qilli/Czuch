#version 450

layout(location=0)in vec3 inColor;
layout(location=1)in vec2 inUV;
layout(location=0)out vec4 outColor;



layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
} sceneData;
layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
 outColor=texture(texSampler,inUV);
}