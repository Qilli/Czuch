#version 460

struct LightData
{
	vec4 positionWithType;
	vec4 color;
	vec4 directionWithRange;
	vec4 spotInnerOuterAngle;
};


struct TileData {
        uint startIndex;
        uint lightCount;
 };

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


layout(std430, binding = 0, set = 1) readonly buffer LightBuffer {
    LightData lights[];
};

layout(std430, binding = 1, set = 1) readonly buffer LightIndexBuffer {
    uint lightIndices[];
};

layout(std430, binding = 2, set = 1) readonly buffer TileDataBuffer {
	ivec4 screenSize;
    TileData tiles[];
};

layout(set = 2, binding = 0) uniform sampler2D texSampler;
layout(set =2, binding = 1) uniform ColorData{
       vec4 color;
} colorData;

const int TILE_SIZE_X = 16;
const int TILE_SIZE_Y = 16;

void main()
{

 ivec2 tileCoord = ivec2( gl_FragCoord.xy/vec2(TILE_SIZE_X , TILE_SIZE_Y));
 int tileIdx = tileCoord.x+tileCoord.y * screenSize.x;

 vec4 temp=texture(texSampler,inUV.xy);
 temp*=colorData.color; 
 temp*=sceneData.ambientColor;
 outColor=vec4(temp.xyz,1);
}