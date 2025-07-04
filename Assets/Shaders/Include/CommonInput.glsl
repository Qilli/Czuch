
layout(push_constant) uniform constants {
    mat4 worldViewProj;
    ivec4 paramsIDObject;
} PushConstants;

layout(set = 0, binding = 0) uniform SceneBlock {
    mat4 view;
    mat4 proj;
    mat4 viewproj;
    vec4 ambientColor;
} sceneData;

