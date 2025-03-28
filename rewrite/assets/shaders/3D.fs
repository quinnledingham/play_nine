#version 450

layout(set = 1, binding = 0) uniform Local {
  vec4 text; // vec4(1=print text show use alpha from texture, 0, 0, 0)
  vec4 color;
  vec4 resolution;
  vec4 time;
} local;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;
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
    float power = 2.2;
    vec4 color = local.color;
    vec4 col = vec4(color.x/255, color.y/255, color.z/255, color.w);
    outColor = col;
}