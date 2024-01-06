#version 450

layout(location=0)in vec2 inPosition;
layout(lcoation=1)in vec3 inColor;
layout(location=0)out vec3 outColor;

void main()
{
gl_position = vec4(inPosition,0.0,1.0);
outColor=inColor;
}