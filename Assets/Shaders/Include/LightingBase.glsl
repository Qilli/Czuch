struct LightData
{
	vec4 positionWithType;
	vec4 color;
	vec4 directionWithRange;
	vec4 spotInnerOuterAngle;
};

struct MaterialData
{
	vec4 albedo;
	vec4 metallicRoughness;
};

struct TileData {
        uint startIndex;
        uint lightCount;
 };

const int TILE_SIZE_X = 32;
const int TILE_SIZE_Y = 32;
const float LINEAR_ATTENUATION = 0.45;

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



float ComputeNormalLightAttenuation(float distance, float range)
{
	return 1.0 / ((distance * distance) + 1.0);
}

float ComputeLightAttenuationWithCutoff(float distance, float range)
{
	float attenuation = 1.0 / ((distance * distance)+ 1.0);
	return attenuation * max(0,(1.0 - distance/range));
}

vec4 ComputeLighting(LightData lightData,MaterialData material, vec4 position, vec3 normal)
{
	float diffuse = 0.0;
	if(lightData.positionWithType.w == 0.0) //directional light
	{
		diffuse = max(dot(normal.xyz,lightData.directionWithRange.xyz), 0.0);
	}
	else if(lightData.positionWithType.w == 1.0) //point light
	{
		vec3 lightDir = normalize(lightData.positionWithType.xyz - position.xyz);
		diffuse = max(dot(normal, lightDir), 0.0);
		float distance = length(lightData.positionWithType.xyz - position.xyz);
		diffuse *= ComputeLightAttenuationWithCutoff(distance,lightData.directionWithRange.w); //attenuation
	}
	else if(lightData.positionWithType.w == 2.0) //spot light
	{
		vec3 fromLightDir = normalize(lightData.positionWithType.xyz - position.xyz);
		float delta = max(dot(fromLightDir, lightData.directionWithRange.xyz),0.0);
		
		if(delta > lightData.spotInnerOuterAngle.y)
		{
			float intensity = min(1.0,(delta - lightData.spotInnerOuterAngle.y) / (lightData.spotInnerOuterAngle.x - lightData.spotInnerOuterAngle.y));
			diffuse = intensity*max(dot(normal.xyz,fromLightDir), 0.0);
			float distance = length(lightData.positionWithType.xyz - position.xyz);
			diffuse *= ComputeLightAttenuationWithCutoff(distance,lightData.directionWithRange.w); //attenuation
		}
		else
		{
			diffuse = 0.0;
		}
	}
	return vec4(lightData.color.xyz*material.albedo.rgb*vec3(diffuse,diffuse,diffuse), 1.0);
}