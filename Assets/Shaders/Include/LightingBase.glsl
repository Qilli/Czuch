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
	return pow(clamp(dot(normal,halfv),0.0,1.0),power);
}


float ComputeShadow(LightData light, vec4 fragPosWorldSpace)
{
	if(light.spotInnerOuterAngle_ShadowMapID.z < 0)
	{
	  return 0.0;
	}

	// Perform perspective divide
	vec4 fragPosLightSpace = light.lightViewProjMatrix * fragPosWorldSpace;
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// Transform to [0,1] range
	projCoords.xy = projCoords.xy * 0.5 + 0.5;
	//flip y coord because texture coord system is upside down
	projCoords.y = 1.0 - projCoords.y;
	// Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	float closestDepth = texture(globalTextures[int(light.spotInnerOuterAngle_ShadowMapID.w)], projCoords.xy).r; 
	// Get depth of current fragment from light's perspective
	float currentDepth = projCoords.z;

	if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z>1.0)
        return 0.0;

	// Check whether current frag pos is in shadow
	float bias = 0.005;
	float shadow = currentDepth-bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}


vec4 ComputeLighting(uint lightIndex,vec4 cameraWorldPos,MaterialData material, vec4 position, vec3 normal,vec2 inUV,vec4 albedo,vec4 metallicSpecularPower)
{
	float diffuse = 0.0;
	float specular = 0.0;
	float attenuation = 1.0;
	float intensity=1.0;
	vec3 cameraDir =normalize(cameraWorldPos.xyz - position.xyz);
	vec3 lightDir = vec3(0,0,0);
	LightData lightData = lights[lightIndex];

	if(lightData.positionWithType.w == 0.0) //directional light
	{
		diffuse = max(dot(normal.xyz,-lightData.directionWithRange.xyz), 0.0);
		lightDir=-lightData.directionWithRange.xyz;
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

		if(delta > lightData.spotInnerOuterAngle_ShadowMapID.y)
		{
			intensity = min(1.0,(delta - lightData.spotInnerOuterAngle_ShadowMapID.y) / (lightData.spotInnerOuterAngle_ShadowMapID.x - lightData.spotInnerOuterAngle_ShadowMapID.y));
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
	if(diffuse>0.0)
	{
	specular =ComputeSpecularLighting(normal.xyz,lightDir, cameraDir,metallicSpecularPower.w);
	}


	vec4 result=vec4(albedo.rgb*diffuse, 1.0);
	result += vec4(specular*metallicSpecularPower.rgb, 1.0);
	result *= vec4(lightData.colorWithIntensity.rgb,1.0f);
	result *= lightData.colorWithIntensity.w;
	result*=attenuation;
	result *= (1.0 - ComputeShadow(lightData, position));
	
	return result;
}