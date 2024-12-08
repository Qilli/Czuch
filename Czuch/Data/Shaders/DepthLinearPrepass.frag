#version 450

layout(location=0)in vec2 inUV;
layout(location=0)out vec4 outColor;
layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 0, binding = 1) uniform PlaneData
{
	float near_plane;
	float far_plane; 
} data;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * data.near_plane * data.far_plane) / (data.far_plane + data.near_plane - z * (data.far_plane - data.near_plane));
}

void main()
{
	vec4 temp=texture(texSampler,inUV);
	float result=LinearizeDepth(temp.x)/ data.far_plane;
	outColor=vec4(result,result,result,1);
}