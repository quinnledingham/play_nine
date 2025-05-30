#version 450

layout(set = 1, binding = 0) uniform Global {
  vec4 resolution;
  vec4 time;
} global;

layout(set = 2, binding = 0) uniform Local {
  vec4 text; // vec4(1=print text show use alpha from texture, 0, 0, 0)
  vec4 color;
  vec4 region; // (offset.x, offset.y, scale.x, scale.y)
} local;

layout(set = 3, binding = 0) uniform sampler2D tex_sampler;

layout(set = 4, binding = 0) uniform Material {
    vec4 ambient; // Ka
    vec4 diffuse; // Kd
    vec4 specular; // Ks (x, y, z) color, Ns (w) specular exponent
} mtl;

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

// mtl file definition
// https://www.loc.gov/preservation/digital/formats/fdd/fdd000508.shtml
void main() {
    vec4 color;
    if (local.text.x == 0.00) {

        float power = 2.2;
        color = local.color;
        vec4 col = vec4(color.x/255, color.y/255, color.z/255, color.w);
        color = col;
        outColor = color;

    } else if (local.text.x == 2.00) {

        color = texture(tex_sampler, fragTexCoord);

        // ambient
        vec3 ambient = mtl.ambient.xyz;

        // diffuse
        vec3 norm = normalize(fragNormal);
        vec3 diffuse = mtl.diffuse.xyz;

        vec3 result = (ambient + diffuse) * color.rgb;

        //vec3 diffuseColor = hasTexture ? texture(tex_sampler, fragTexCoord).rgb : mtl.diffuse.xyz;
        //vec3 diffuse = diff * diffuseColor;

        outColor = vec4(result, color.a);

    } else if (local.text.x == 3.00) { // texture atlas

        vec2 uv_offset = local.region.xy;
        vec2 uv_scale = local.region.zw;
        vec2 remapped_uv = fragTexCoord * uv_scale + uv_offset;

        outColor = texture(tex_sampler, remapped_uv);

    }
}