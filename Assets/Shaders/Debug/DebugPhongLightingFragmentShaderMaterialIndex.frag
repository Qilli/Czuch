#version 460
#extension GL_GOOGLE_include_directive : require
#include "Common.glsl"
#include "CommonInput.glsl"
#include "LightingBase.glsl"

layout(location=0)in vec4 inColor;
layout(location=1)in vec4 inUV;
layout(location=2)in vec4 inPos;
layout(location=3)in vec4 inNormal;
layout(location=0)out vec4 outColor;


layout(std430, binding = 0, set = 1) readonly buffer LightBuffer {
    LightData lights[];
};

layout(std430, binding = 1, set = 1) readonly buffer LightIndexBuffer {
	ivec4 screenSize;
    uint lightIndices[];
};

layout(std430, binding = 2, set = 1) readonly buffer TileDataBuffer {
    TileData tiles[];
};

void main()
{
    float materialIndex = inNormal.w; // Use the w component of the normal to store the material index
    float brightness = materialIndex / 255.0; // Normalize the material index to a range of 0.0 to 1.0
    outColor = vec4(1.0-brightness, brightness, brightness, 1.0); // Output the color to show debug information about current material index
}