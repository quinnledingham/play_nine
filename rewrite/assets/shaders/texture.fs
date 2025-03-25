#version 450

layout(set = 1, binding = 0) uniform sampler2D texSampler;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 3) flat in int fragIndex;

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

void main() {
  outColor = texture(texSampler, fragTexCoord);
  //outColor = to_sRGB(outColor);
}
