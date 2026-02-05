#ifndef LIGHTING_BASE_GLSL
#define LIGHTING_BASE_GLSL

// Include the new decoupled modules
#include "GlobalResources.glsl"
#include "LightingData.glsl"
#include "LightingMath.glsl"
#include "Shadows.glsl"

vec4 ComputeLighting(uint lightIndex, vec4 cameraWorldPos, MaterialData material, vec4 position, vec3 normal, vec2 inUV, vec4 albedo, vec4 metallicSpecularPower)
{
    float diffuse = 0.0;
    float specular = 0.0;
    float attenuation = 1.0;
    float intensity = 1.0;
    
    vec3 cameraDir = normalize(cameraWorldPos.xyz - position.xyz);
    vec3 lightDir = vec3(0,0,0);
    LightData lightData = lights[lightIndex];

    if(lightData.positionWithType.w == 0.0) // directional light
    {
        diffuse = max(dot(normal.xyz, -lightData.directionWithRange.xyz), 0.0);
        lightDir = -lightData.directionWithRange.xyz;
    }
    else if(lightData.positionWithType.w == 1.0) // point light
    {
        lightDir = normalize(lightData.positionWithType.xyz - position.xyz);
        diffuse = max(dot(normal, lightDir), 0.0);
        float distance = length(lightData.positionWithType.xyz - position.xyz);

        attenuation = ComputeLightAttenuationWithCutoff(distance, lightData.directionWithRange.w);
    }
    else if(lightData.positionWithType.w == 2.0) // spot light
    {
        vec3 fromLightDir = normalize(position.xyz - lightData.positionWithType.xyz);
        float delta = max(dot(fromLightDir, lightData.directionWithRange.xyz), 0.0);
        
        if(delta > lightData.spotInnerOuterAngle_ShadowMapID.y)
        {
            intensity = min(1.0, (delta - lightData.spotInnerOuterAngle_ShadowMapID.y) / (lightData.spotInnerOuterAngle_ShadowMapID.x - lightData.spotInnerOuterAngle_ShadowMapID.y));
            diffuse = intensity * max(dot(normal.xyz, -fromLightDir), 0.0);
            float distance = length(lightData.positionWithType.xyz - position.xyz);

            float atten = ComputeLightAttenuationWithCutoff(distance, lightData.directionWithRange.w);
            diffuse *= atten;

            lightDir = fromLightDir;
            attenuation = atten;
        }
        else
        {
            diffuse = 0.0;
        }
    }
    
    if(diffuse > 0.0)
    {
        specular = ComputeSpecularLighting(normal.xyz, lightDir, cameraDir, metallicSpecularPower.w);
    }

    vec4 result = vec4(albedo.rgb * diffuse, 1.0);
    result += vec4(specular * metallicSpecularPower.rgb, 1.0);
    result *= vec4(lightData.colorWithIntensity.rgb, 1.0f);
    result *= lightData.colorWithIntensity.w;
    result *= attenuation;
    result *= (1.0 - ComputeShadow(lightData, position));
    
    return result;
}

#endif