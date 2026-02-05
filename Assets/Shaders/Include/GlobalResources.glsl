#ifndef GLOBAL_RESOURCES_GLSL
#define GLOBAL_RESOURCES_GLSL

// Binding 0: The array of textures (No samplers attached)
layout(set = 2, binding = 0) uniform texture2D globalTextures[1024];

// Binding 1: The one sampler we use for everything(linear)
layout(set = 2, binding = 1) uniform sampler globalSampler;

// Binding 2: point filtering sampler
layout(set = 2, binding = 2) uniform sampler globalPointSampler;

// --- Helper Functions ---
vec4 SampleGlobalTexture(int index, vec2 uv)
{
    // 1. Get the texture object using nonuniformEXT(we inform )
    // 2. Combine it with the globalSampler using sampler2D() constructor
    return texture(sampler2D(globalTextures[nonuniformEXT(index)], globalSampler), uv);
}

vec4 SampleShadowMap(int index, vec2 uv)
{
    // 1. Get the texture object using nonuniformEXT
    // 2. Combine it with the globalPointSampler using sampler2D() constructor
    return texture(sampler2D(globalTextures[nonuniformEXT(index)], globalSampler), uv);
}

ivec2 GlobalTextureSize(int index)
{
    return textureSize(sampler2D(globalTextures[nonuniformEXT(index)], globalSampler),0);
}

#endif