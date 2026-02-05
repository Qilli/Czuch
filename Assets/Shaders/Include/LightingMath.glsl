#ifndef LIGHTING_MATH_GLSL
#define LIGHTING_MATH_GLSL

const float LINEAR_ATTENUATION = 0.45;

float ComputeNormalLightAttenuation(float distance, float range)
{
    return 1.0 / ((distance * distance) + 1.0);
}

float ComputeLightAttenuationWithCutoff(float distance, float range)
{
    float attenuation = 1.0 / ((distance * distance)+ 1.0);
    return attenuation * max(0,(1.0 - distance/range));
}

float ComputeSpecularLighting(vec3 normal, vec3 lightingDir, vec3 view, float power)
{
    vec3 halfv = normalize(view+lightingDir);
    return pow(clamp(dot(normal,halfv),0.0,1.0),power);
}

#endif