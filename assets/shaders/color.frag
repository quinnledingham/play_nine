#version 450

layout(set = 1, binding = 1) uniform Color {
    vec4 c;
} color;

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    //vec3 norm_color = vec3(color.c.x/255, color.c.y/255, color.c.z/255);
    //outColor = vec4(norm_color, color.c.z);
    outColor = vec4(color.c.x/255, color.c.y/255, color.c.z/255, color.c.w);
}