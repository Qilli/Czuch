#version 460
#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require
#include "Common.glsl"
#include "CommonInput.glsl"
#include "LightingBase.glsl"

layout(location=0)in vec4 inColor;
layout(location=1)in vec4 inUV;
layout(location=2)in vec4 inPos;
layout(location=3)in vec4 inNormal;
layout(location=0)out vec4 outColor;

void main()
{
	 ivec2 tileCoord = ivec2( gl_FragCoord.xy/vec2(TILE_SIZE_X , TILE_SIZE_Y));
	 int tileIndex = tileCoord.x+tileCoord.y * screenSize.z;
	 ivec2 lightListRange = ivec2(tiles[tileIndex].startIndex,tiles[tileIndex].lightCount);
	 outColor=vec4(0,0,0,1);
	 int count=lightListRange.x+lightListRange.y;
	 vec3 normal =normalize(inNormal.xyz);
	 RenderObject obj = renderObjects[PushConstants.paramsIDObject.x];
	 MaterialData materialData = materials[obj.materialIndexAndFlags.x];

	for( int i = lightListRange.x; i < count; ++i )
	{
		LightData lightData=lights[lightIndices[i]];
		outColor+=ComputeLighting(lightData,sceneData.cameraWorldPos,materialData,inPos,normal,inUV.xy,sceneData.ambientColor.xyz);
	}
}