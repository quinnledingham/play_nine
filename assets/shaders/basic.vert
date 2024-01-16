#version 450

layout(binding = 0) uniform Scene {
    mat4 view;
    mat4 projection;
} scene;

layout(binding = 2) uniform Object {
    mat4 model;
} object;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = scene.projection * scene.view * object.model * vec4(inPosition, 1.0);
    fragTexCoord = inTexCoord;
}