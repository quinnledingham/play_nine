inline void
init_basic_vert_layout(Shader *shader) {
    shader->layout_sets[0].descriptors[0] = Descriptor(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, sizeof(Scene), descriptor_scope::GLOBAL);
    shader->layout_sets[0].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 0);

    shader->layout_sets[2].descriptors[0].Push_Descriptor(SHADER_STAGE_VERTEX, sizeof(Matrix_4x4), descriptor_scope::LOCAL);
    shader->layout_sets[2].push_constant_count = 1;
}

inline void
init_basic_frag_layout(Shader *shader) {
    shader->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Light), descriptor_scope::GLOBAL);
    shader->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 1);

    shader->layout_sets[3].descriptors[0] = Descriptor(2, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, descriptor_scope::GLOBAL);
    shader->layout_sets[3].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 3);
}

inline void
init_color3D_frag_layout(Shader *shader) {
    shader->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Light), descriptor_scope::GLOBAL);
    shader->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 1);

    shader->layout_sets[3].descriptors[0] = Descriptor(2, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Vector4), descriptor_scope::GLOBAL);
    shader->layout_sets[3].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 3);
}

inline void
init_text_frag_layout(Shader *shader) {
    shader->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, descriptor_scope::GLOBAL);
    shader->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 1);

    shader->layout_sets[3].descriptors[0] = Descriptor(2, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Vector4), descriptor_scope::GLOBAL);
    shader->layout_sets[3].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 3);
}

inline void
init_color_frag_layout(Shader *shader) {
    shader->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Vector4), descriptor_scope::GLOBAL);
    shader->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 1);
}

inline void
init_texture_frag_layout(Shader *shader) {
    shader->layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, descriptor_scope::GLOBAL);
    shader->layout_sets[1].descriptors_count = 1;
    render_create_descriptor_pool(shader, shader->max_sets, 1);
}