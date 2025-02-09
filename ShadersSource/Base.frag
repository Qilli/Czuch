#version 450

layout(location=0)in vec4 inColor;
layout(location=1)in vec4 inUV;
layout(location=2)in vec4 inNormal;
layout(location=0)out vec4 outColor;

layout(set = 0, binding = 0) uniform  SceneData{   

	mat4 view;
	mat4 proj;
	mat4 viewproj;
	vec4 ambientColor;
} sceneData;
layout(set = 1, binding = 0) uniform sampler2D texSampler;
layout(set =1, binding = 1) uniform ColorData{
       vec4 color;
} colorData;

void main()
{
 vec4 temp=texture(texSampler,inUV.xy);
 temp*=colorData.color; 
 temp*=sceneData.ambientColor;
 outColor=vec4(temp.xyz,1);
}