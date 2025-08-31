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
	ivec4 albedoMetallicTextures;
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

layout(set = 2, binding = 0) uniform sampler2D globalTextures[];

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

vec4 ComputeLighting(LightData lightData,vec4 cameraWorldPos,MaterialData material, vec4 position, vec3 normal,vec2 inUV,vec3 ambient)
{
	float diffuse = 0.0;
	float specular = 0.0;
	float attenuation = 1.0;
	float intensity=1.0;
	vec3 cameraDir =normalize(cameraWorldPos.xyz - position.xyz);
	vec3 lightDir = vec3(0,0,0);

	vec3 albedo = material.albedoMetallicTextures.x == -1 ? material.albedo.rgb : texture(globalTextures[material.albedoMetallicTextures.x],inUV.xy).rgb;
	vec3 metallicRoughness = material.albedoMetallicTextures.y == -1 ? material.metallicRoughness.rgb : texture(globalTextures[material.albedoMetallicTextures.y],inUV.xy).rgb;

	if(lightData.positionWithType.w == 0.0) //directional light
	{
		diffuse = max(dot(normal.xyz,-lightData.directionWithRange.xyz), 0.0);
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
		vec3 fromLightDir = normalize( position.xyz-lightData.positionWithType.xyz);
		float delta = max(dot(fromLightDir,lightData.directionWithRange.xyz),0.0);

		if(delta > lightData.spotInnerOuterAngle.y)
		{
			intensity = min(1.0,(delta - lightData.spotInnerOuterAngle.y) / (lightData.spotInnerOuterAngle.x - lightData.spotInnerOuterAngle.y));
			diffuse = intensity*max(dot(normal.xyz,-fromLightDir), 0.0);
			float distance = length(lightData.positionWithType.xyz - position.xyz);

			float atten=ComputeLightAttenuationWithCutoff(distance,lightData.directionWithRange.w); //attenuation
			diffuse *= atten;

			lightDir = fromLightDir;
			attenuation = atten;
		}
		else
		{
			diffuse = 0.0;
		}
	}
	if(diffuse>0)
	{
	specular = ComputeSpecularLighting(normal.xyz,-lightDir, cameraDir, material.metallicRoughness.w);
	}


	vec4 result=vec4(lightData.color.xyz*albedo*diffuse, 1.0);
	result += vec4(lightData.color.xyz * specular *metallicRoughness.rgb, 1.0);
	result*=attenuation;
	result+=vec4(ambient*albedo,1.0);
	
	return result;
}