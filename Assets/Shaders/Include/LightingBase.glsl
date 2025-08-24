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

float ComputeSpecularLighting(vec3 normal, vec3 lightingDir, vec3 view,float power)
{
	vec3 halfv = normalize(view+lightingDir);
	return clamp(pow(dot(normal,halfv),power),0.0,1.0);
}

vec4 ComputeLighting(LightData lightData,vec4 cameraWorldPos,MaterialData material, vec4 position, vec3 normal)
{
	float diffuse = 0.0;
	float specular = 1.0;
	float attenuation = 1.0;
	float intensity=1.0;
	vec3 cameraDir =normalize(cameraWorldPos.xyz - position.xyz);
	vec3 lightDir = vec3(0,0,0);

	if(lightData.positionWithType.w == 0.0) //directional light
	{
		diffuse = max(dot(normal.xyz,lightData.directionWithRange.xyz), 0.0);
		lightDir=lightData.directionWithRange.xyz;
	}
	else if(lightData.positionWithType.w == 1.0) //point light
	{
		lightDir = normalize(lightData.positionWithType.xyz - position.xyz);
		diffuse = max(dot(normal, lightDir), 0.0);
		float distance = length(lightData.positionWithType.xyz - position.xyz);

		attenuation=ComputeLightAttenuationWithCutoff(distance,lightData.directionWithRange.w); //attenuation
	}
	else if(lightData.positionWithType.w == 2.0) //spot light
	{
		vec3 fromLightDir = normalize(lightData.positionWithType.xyz - position.xyz);
		float delta = max(dot(fromLightDir, lightData.directionWithRange.xyz),0.0);
		
		if(delta > lightData.spotInnerOuterAngle.y)
		{
			intensity = min(1.0,(delta - lightData.spotInnerOuterAngle.y) / (lightData.spotInnerOuterAngle.x - lightData.spotInnerOuterAngle.y));
			diffuse = intensity*max(dot(normal.xyz,fromLightDir), 0.0);
			float distance = length(lightData.positionWithType.xyz - position.xyz);

			
			float atten=ComputeLightAttenuationWithCutoff(distance,lightData.directionWithRange.w); //attenuation
			diffuse *= atten;

			lightDir = fromLightDir;
			attenuation = atten;
		}
		else
		{
			diffuse = 0.0;
			specular = 0.0;
		}
	}

	specular *= ComputeSpecularLighting(normal.xyz,lightDir, cameraDir, material.metallicRoughness.w);
	return vec4(lightData.color.xyz*(material.albedo.rgb*diffuse + specular * material.metallicRoughness.rgb)*attenuation, 1.0);
}