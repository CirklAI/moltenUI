#version 450

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D fontAtlas;

void main() {
    float alpha = texture(fontAtlas, inUV).r;

    if (alpha < 0.01) discard;

    outColor = vec4(inColor.rgb, inColor.a * alpha);
}
