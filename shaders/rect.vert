#version 450
layout(location = 0) out vec2 fragCoord;

layout(push_constant) uniform Constants {
    vec2 pos;
    vec2 size;
} PC;

vec2 positions[6] = vec2[](
    vec2(-1.0, -1.0), vec2(1.0, -1.0), vec2(1.0, 1.0),
    vec2(-1.0, -1.0), vec2(1.0, 1.0), vec2(-1.0, 1.0)
);

void main() {
    vec2 p = positions[gl_VertexIndex];
    fragCoord = p;
    gl_Position = vec4(p * PC.size + PC.pos, 0.0, 1.0);
}
