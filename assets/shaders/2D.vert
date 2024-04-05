#version 450

layout(set = 0, binding = 0) uniform Scene {
    mat4 view;
    mat4 projection;
} scene;

layout(push_constant, std430) uniform Object {
    mat4 model;
    int index;
} object;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out int fragIndex;

void main() {
    gl_Position = scene.projection * scene.view * object.model * vec4(inPosition, 0.0, 1.0);
    fragTexCoord = inTexCoord;
    fragIndex = object.index;
}