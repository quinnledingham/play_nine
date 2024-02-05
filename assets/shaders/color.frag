#version 450

layout(set = 1, binding = 1) uniform Color {
    vec4 c;
} color;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = color.c;
}