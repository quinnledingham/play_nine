#version 450

layout(set = 1, binding = 0) uniform Local {
  vec4 text; // vec4(1=print text show use alpha from texture, 0, 0, 0)
  vec4 color;
  vec4 resolution;
  vec4 time;
} local;

layout(set = 2, binding = 0) uniform sampler2D tex_sampler;

layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

float to_sRGB(float RGB) {
	float sRGB = 0.0;
	if (RGB >= 1.0)
		sRGB = 1.0;
	else if (RGB <= 0.0)
		sRGB = 0.0;
	else if (RGB < 0.0031308)
		sRGB = 12.92 * RGB;
	else
		sRGB = 1.055 * pow(RGB, 0.41666) - 0.055;
	return sRGB;
}

vec4 to_sRGB(vec4 RGB) {
	return vec4(to_sRGB(RGB.r), to_sRGB(RGB.g), to_sRGB(RGB.b), RGB.a);	
}

// Converts a color from sRGB gamma to linear light gamma
// https://gamedev.stackexchange.com/questions/92015/optimized-linear-to-srgb-glsl
vec4 to_linear(vec4 sRGB) {
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

void main() {
    vec3 norm_color = vec3(local.color.x/255, local.color.y/255, local.color.z/255);
    vec4 color = vec4(norm_color, local.color.w);

    if (local.text.x == 1.00) {
	    float alpha = texture(tex_sampler, fragTexCoord).r * local.color.a;
	    color = vec4(norm_color, alpha);
	    //color = texture(tex_sampler, fragTexCoord);
    } else if (local.text.x == 2.00) {
    	color = texture(tex_sampler, fragTexCoord);
    }

    if (local.text.x != 2.00)
    	color = to_linear(color);
	//color = to_sRGB(color);

    outColor = color;
}
