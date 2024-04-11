#version 450

layout(set = 1, binding = 1) uniform Color {
    vec4 c;
} color;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 3) flat in int fragIndex;

layout(location = 0) out vec4 outColor;

// Converts a color from sRGB gamma to linear light gamma
// https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
vec4 to_linear(vec4 sRGB) {
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

void main() {
    vec3 norm_color = vec3(color.c.x/255, color.c.y/255, color.c.z/255);
    vec4 color = vec4(norm_color, color.c.w);
    color = to_linear(color);
    outColor = color;
}