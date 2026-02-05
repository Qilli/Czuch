#ifndef LIGHTING_DATA_GLSL
#define LIGHTING_DATA_GLSL

#include "CommonInput.glsl" // Required for LightData struct

struct MaterialData
{
    vec4 albedo;
    vec4 metallicSpecularPower;
    ivec4 albedoMetallicTextures;
};

struct TileData {
    uint startIndex;
    uint lightCount;
};

const int TILE_SIZE_X = 32;
const int TILE_SIZE_Y = 32;

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

layout(std430, binding = 3, set = 1) readonly buffer MaterialsDataBuffer {
    MaterialData materials[];
};

#endif