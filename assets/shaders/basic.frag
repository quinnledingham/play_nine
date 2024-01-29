#version 450

layout(set = 1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 tex = texture(texSampler, fragTexCoord).rgb;

    vec3 light_position = vec3(2.0, 5.0, 0.0);
    vec3 light_color = vec3(1.0, 1.0, 1.0);

    // ambient
    float ambient_strength = 0.4;
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
}