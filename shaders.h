inline void
init_layouts(Layout layouts[10], Bitmap *bitmap) {
    layouts[0].bindings[0] = Layout_Binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert, basic.vert
    layouts[1].bindings[0] = Layout_Binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Light)); // basic.frag, color3D.frag
    layouts[2].bindings[0] = Layout_Binding(2, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, TEXTURE_ARRAY_SIZE); // basic.frag, text.frag
    layouts[3].bindings[0] = Layout_Binding(0, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, TEXTURE_ARRAY_SIZE); // texture.frag
    layouts[4].bindings[0] = Layout_Binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color.frag, text.frag
    layouts[5].bindings[0] = Layout_Binding(2, DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color3D.frag

    layouts[0].set_number = 0;
    layouts[1].set_number = 1;
    layouts[2].set_number = 2;
    layouts[3].set_number = 1;
    layouts[4].set_number = 1;
    layouts[5].set_number = 2;

    for (u32 i = 0; i < 6; i++) {
        vulkan_create_set_layout(&layouts[i]);
        vulkan_allocate_descriptor_set(&layouts[i]);
        vulkan_init_layout_offsets(&layouts[i], bitmap);
    }
}

inline void
init_basic_vert_layout(Layout_Set *set, Layout layouts[10]) {
    set->add_layout(&layouts[0]);

    set->add_push(SHADER_STAGE_VERTEX, sizeof(Object));
    set->enabled = true;
}

inline void
init_basic_frag_layout(Shader *shader, Layout layouts[10]) {
    shader->set.add_layout(&layouts[1]);
    shader->set.add_layout(&layouts[2]);
}

inline void
init_color3D_frag_layout(Shader *shader, Layout layouts[10]) {
    shader->set.add_layout(&layouts[1]);
    shader->set.add_layout(&layouts[5]);
}

inline void
init_text_frag_layout(Shader *shader, Layout layouts[10]) {
    shader->set.add_layout(&layouts[4]);
    shader->set.add_layout(&layouts[2]);
}

inline void
init_color_frag_layout(Shader *shader, Layout layouts[10]) {
    shader->set.add_layout(&layouts[4]);
}

inline void
init_texture_frag_layout(Shader *shader, Layout layouts[10]) {
    shader->set.add_layout(&layouts[3]);
}