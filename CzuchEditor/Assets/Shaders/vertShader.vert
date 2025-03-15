#version 460

layout(location=0) in vec3 inPosition;
layout(location=1) in vec4 inColor;
layout(location=2) in vec4 inUV;
layout(location=3) in vec4 inNormal;

layout(location=0) out vec4 outColor;
layout(location=1) out vec4 outUV;
layout(location=2) out vec4 outNormal;

layout(set = 0, binding = 0) uniform SceneData {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 ambientColor;
} sceneData;

layout(push_constant) uniform constants {
    mat4 worldViewProj;
} PushConstants;

void main() {
    gl_Position = PushConstants.worldViewProj * vec4(inPosition.xyz, 1.0);
    outColor = inColor;
    outUV = inUV;
    outNormal = inNormal;
}