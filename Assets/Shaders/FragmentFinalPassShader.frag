#version 450

layout(location=0)in vec2 inUV;
layout(location=0)out vec4 outColor;
layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main()
{
    vec3 hdrColor = texture(texSampler, inUV).rgb;

    // Apply the Reinhard tonemapping operator.
    // This compresses the HDR values into the [0, 1] range.
    vec3 mappedColor = hdrColor / (1.0 + hdrColor);
    
    // The final result is in linear space, which is what the sRGB framebuffer expects.
    outColor = vec4(mappedColor, 1.0);
}