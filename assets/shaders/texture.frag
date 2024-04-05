#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler[64];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in int fragIndex;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler[fragIndex], fragTexCoord);
}