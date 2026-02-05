#ifndef SHADOWS_GLSL
#define SHADOWS_GLSL

#include "CommonInput.glsl"
#include "GlobalResources.glsl"

float ComputeShadow(LightData light, vec4 fragPosWorldSpace)
{
    if(light.spotInnerOuterAngle_ShadowMapID.z < 0)
    {
      return 0.0;
    }

    vec2 texelSize = 1.0 / GlobalTextureSize(int(light.spotInnerOuterAngle_ShadowMapID.w));
    float shadow=0.0;
    vec4 fragPosLightSpace = light.lightViewProjMatrix * fragPosWorldSpace;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords.xy = projCoords.xy * 0.5 + 0.5;
    
    // flip y coord because texture coord system is upside down
    projCoords.y = 1.0 - projCoords.y;
    
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0)
        return 0.0;

    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    
    // Check whether current frag pos is in shadow
    float bias = 0.0005;

    // PCF
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float closestDepth = SampleShadowMap(int(light.spotInnerOuterAngle_ShadowMapID.w), projCoords.xy + vec2(x,y) * texelSize).r;
            if(currentDepth - bias > closestDepth)
            {
                shadow += 1.0;
            }
        }
    }

    return clamp(shadow /= 9.0, 0.0, 1.0);
}

#endif