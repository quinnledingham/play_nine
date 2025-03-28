#version 450

layout(set = 1, binding = 1) uniform Light_Source {
    vec4 position;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 color;
    bool enabled;
} light;

layout(set = 2, binding = 2) uniform Color {
    vec4 c;
} color;

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
    vec4 col = vec4(color.c.x/255, color.c.y/255, color.c.z/255, color.c.w);
    //col = to_linear(col);

    vec3 light_position = light.position.rgb;
    vec3 light_color = light.color.rgb;

    // ambient
    float ambient_strength = light.ambient.r;
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
    vec3 result = (ambient + diffuse) * col.rgb;

    if (light.enabled)
        outColor = vec4(result, color.c.w);
    else
        outColor = col;
}