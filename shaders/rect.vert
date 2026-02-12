#version 450

layout(push_constant) uniform Constants {
    vec2 pos;
    vec2 size;
    vec4 color;
    vec2 resolution;
} PC;

layout(location = 0) out vec2 fragCoord;
layout(location = 1) out vec4 outColor;

vec2 positions[6] = vec2[](
    vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0),
    vec2(0.0, 0.0), vec2(1.0, 1.0), vec2(0.0, 1.0)
);

void main() {
    vec2 p = positions[gl_VertexIndex];
    vec2 pixelPos = PC.pos + (p * PC.size);
    vec2 ndc = (pixelPos / PC.resolution) * 2.0 - 1.0;
    fragCoord = p * PC.size;
    outColor = PC.color;
    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);
}
