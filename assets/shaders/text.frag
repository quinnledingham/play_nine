#version 450

layout(set = 1, binding = 2) uniform sampler2D texSampler;
layout(set = 1, binding = 3) uniform Text {
    vec4 color;
} text;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
//    outColor = texture(texSampler, fragTexCoord);

    vec3 norm_text_color = vec3(text.color.x/255, text.color.y/255, text.color.z/255);
    float alpha = texture(texSampler, fragTexCoord).r * text.color.a;
    vec4 tex = vec4(1.0, 1.0, 1.0, alpha);
    outColor = vec4(norm_text_color, 1.0) * tex;

}