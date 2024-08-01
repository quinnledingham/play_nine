#version 450

layout(set = 1, binding = 0) uniform sampler2D tex[16];

layout(location = 1) in vec2 frag_tex_coord;
layout(location = 3) flat in int frag_index;

layout(location = 0) out vec4 outColor;

void main() {
    int pos_x = frag_index % 10;
    int pos_y = frag_index / 10;
    float width = 1.0f / 10.0f;
    float height = 1.0f / 9.0f;
    vec2 tex_coords = vec2((frag_tex_coord.x / 10.0f) + (width * pos_x), (frag_tex_coord.y / 9.0f) + (height * pos_y));
    outColor = texture(tex[0], tex_coords);
}
