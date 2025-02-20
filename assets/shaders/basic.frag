#version 450

layout(set = 1, binding = 1) uniform Light_Source {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 color;
    bool enabled;
} light;

layout(set = 2, binding = 2) uniform sampler2D texSampler[16];

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;
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
    vec3 tex = texture(texSampler[fragIndex], fragTexCoord).rgb;

    vec3 light_position = light.position.rgb;
    vec3 light_color = light.color.rgb;

    // ambient
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    // diffuse 
    vec3 norm = normalize(fragNormal);
    vec3 lightDir = normalize(light_position - fragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * light_color;
    
    // specular
/*
    float specular_strength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
  */      
    vec3 result = (ambient + diffuse) * tex;

    outColor = vec4(result, 1.0);
	outColor = to_sRGB(outColor);
}