#version 450

layout(location=0)in vec3 inPosition;

layout( push_constant ) uniform constants
{
 mat4 worldViewProj;
} PushConstants;

void main()
{
	gl_Position = PushConstants.worldViewProj*vec4(inPosition.xyz,1.0);
}