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

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set =2, binding = 1) uniform ColorData{
       vec4 color;
} colorData;

void main()
{
	 vec4 diffuseTex=texture(texSampler,inUV.xy);
	 ivec2 tileCoord = ivec2( gl_FragCoord.xy/vec2(TILE_SIZE_X , TILE_SIZE_Y));
	 int tileIndex = tileCoord.x+tileCoord.y * screenSize.z;
	 ivec2 lightListRange = ivec2(tiles[tileIndex].startIndex,tiles[tileIndex].lightCount);
	 outColor=diffuseTex*sceneData.ambientColor;
	 int count=lightListRange.x+lightListRange.y;
	 vec3 normal =normalize(inNormal.xyz);


	 for( int i = lightListRange.x; i < count; ++i )
	{
		LightData lightData=lights[lightIndices[i]];
		outColor+=diffuseTex* ComputeLighting(lightData,inPos,normal);
	}
}