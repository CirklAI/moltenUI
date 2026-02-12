#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants {
    vec2 resolution;
} push;

void main() {
    vec2 normalized = (inPosition / push.resolution) * 2.0 - 1.0;
    gl_Position = vec4(normalized, 0.0, 1.0);
    fragTexCoord = inTexCoord;
}
