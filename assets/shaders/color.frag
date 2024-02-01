#version 450

layout(set = 1, binding = 3) uniform Color {
    vec4 color;
} color;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = color.color;
}