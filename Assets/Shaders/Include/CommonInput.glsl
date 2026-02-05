#ifndef COMMON_INPUT_GLSL
#define COMMON_INPUT_GLSL

layout(push_constant) uniform constants {
    mat4 worldViewProj;
    ivec4 paramsIDObject;
} PushConstants;

layout(set = 0, binding = 0) uniform SceneBlock {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 cameraWorldPos;
    vec4 ambientColor;
} sceneData;

struct LightData
{
	vec4 positionWithType;
	vec4 colorWithIntensity;
	vec4 directionWithRange;
	vec4 spotInnerOuterAngle_ShadowMapID;
	mat4 lightViewProjMatrix;
};

#endif