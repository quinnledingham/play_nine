#version 450

layout(set = 1, binding = 1) uniform Text {
    vec4 color;
} text;
layout(set = 2, binding = 2) uniform sampler2D texSampler[64];

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) flat in int fragIndex;

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
    vec4 norm_text_color = vec4(text.color.x/255, text.color.y/255, text.color.z/255, text.color.w);
    norm_text_color = to_linear(norm_text_color);

    float alpha = texture(texSampler[fragIndex], fragTexCoord).r * text.color.a;
    vec4 tex = vec4(1.0, 1.0, 1.0, alpha);
    outColor = norm_text_color * tex;
}