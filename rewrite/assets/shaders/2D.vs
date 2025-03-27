#version 450

layout(set = 0, binding = 0) uniform Scene {
    mat4 view;
    mat4 projection;
} scene;

layout(push_constant, std430) uniform Object {
    mat4 model;
    int index;
} object;

layout(location = 0) in vec4 vertex;

layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = scene.projection * scene.view * object.model * vec4(vertex.xy, 0.0, 1.0);
    fragTexCoord = vertex.zw;
}