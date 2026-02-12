#version 450

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

layout(push_constant) uniform Constants {
    vec2 pos;
    vec2 size;
    vec4 color;
    vec4 uvRect;
} PC;

// todo: don't
const vec2 screenRes = vec2(800.0, 600.0);

vec2 positions[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

void main() {
    vec2 p = positions[gl_VertexIndex];

    outUV = PC.uvRect.xy + (p * PC.uvRect.zw);
    outColor = PC.color;

    vec2 pixelPos = PC.pos + (p * PC.size);
    vec2 ndc = (pixelPos / screenRes) * 2.0 - 1.0;
    
    gl_Position = vec4(ndc, 0.0, 1.0);
}
