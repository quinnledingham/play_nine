enum GFX_Layout_IDs {
    GFX_ID_SCENE,
    GFX_ID_LIGHT,
    GFX_ID_TEXT,
    GFX_ID_TEXTURE,
    GFX_ID_COLOR_2D,
    GFX_ID_COLOR_3D,
    
    GFX_ID_RAY_VERTEX,
    GFX_ID_RAY_TRIANGLE,
    GFX_ID_RAY_INTERSECTION,
    GFX_ID_RAY_MODELS,
    GFX_ID_BASIC_TEXTURE,

    GFX_ID_COUNT
};

inline void
init_layouts(Layout *layouts, Bitmap *bitmap) {
    //layouts[GFX_ID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX,)
    
    layouts[GFX_ID_SCENE].add_binding(Layout_Binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene))); // 2D.vert, basic.vert
    layouts[GFX_ID_LIGHT].add_binding(Layout_Binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Light))); // basic.frag, color3D.frag
    layouts[GFX_ID_TEXT].add_binding(Layout_Binding(2, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, TEXTURE_ARRAY_SIZE)); // text.frag
    layouts[GFX_ID_TEXTURE].add_binding(Layout_Binding(0, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, TEXTURE_ARRAY_SIZE)); // texture.frag
    layouts[4].bindings[0] = Layout_Binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color.frag, text.frag
    layouts[5].bindings[0] = Layout_Binding(2, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color3D.frag

    layouts[6].bindings[0] = Layout_Binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_COMPUTE, 1, sizeof(Ray_v4)); // ray.comp
    layouts[7].bindings[0] = Layout_Binding(1, DESCRIPTOR_TYPE_STORAGE_BUFFER, SHADER_STAGE_COMPUTE, 1, sizeof(Triangle_v4)); // ray.comp
    layouts[8].bindings[0] = Layout_Binding(2, DESCRIPTOR_TYPE_STORAGE_BUFFER, SHADER_STAGE_COMPUTE, 1, sizeof(Ray_Intersection)); // ray.comp
    layouts[9].bindings[0] = Layout_Binding(3, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_COMPUTE, 1, 10 * sizeof(Matrix_4x4)); // ray.comp
    layouts[10].bindings[0] = Layout_Binding(2, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, TEXTURE_ARRAY_SIZE); // basic.frag
    
    layouts[0].set_number = 0;
    layouts[1].set_number = 1;
    layouts[2].set_number = 2;
    layouts[3].set_number = 1;
    layouts[4].set_number = 1;
    layouts[5].set_number = 2;

    layouts[6].set_number = 0;
    layouts[7].set_number = 1;
    layouts[8].set_number = 2;
    layouts[9].set_number = 3;
    layouts[10].set_number = 2;

    for (u32 i = 0; i < GFX_ID_COUNT; i++) {
        layouts[i].id = i;
        gfx.create_set_layout(&layouts[i]);
        gfx.allocate_descriptor_set(&layouts[i]);
        gfx.init_layout_offsets(&layouts[i], bitmap);
    }
}

inline void
init_basic_vert_layout(Layout_Set *set, Layout layouts[11]) {
    set->add_layout(&layouts[0]);

    set->add_push(SHADER_STAGE_VERTEX, sizeof(Object));
}

inline void
init_basic_frag_layout(Shader *shader, Layout layouts[11]) {
    shader->set.add_layout(&layouts[1]);
    shader->set.add_layout(&layouts[2]);
}

inline void
init_color3D_frag_layout(Shader *shader, Layout layouts[11]) {
    shader->set.add_layout(&layouts[1]);
    shader->set.add_layout(&layouts[5]);
}

inline void
init_text_frag_layout(Shader *shader, Layout layouts[11]) {
    shader->set.add_layout(&layouts[4]);
    shader->set.add_layout(&layouts[2]);
}

inline void
init_color_frag_layout(Shader *shader, Layout layouts[11]) {
    shader->set.add_layout(&layouts[4]);
}

inline void
init_texture_frag_layout(Shader *shader, Layout layouts[11]) {
    shader->set.add_layout(&layouts[3]);
}

inline void
init_ray_comp_layout(Layout_Set *set, Layout layouts[11]) {
    set->add_layout(&layouts[6]);
    set->add_layout(&layouts[7]);
    set->add_layout(&layouts[8]);
    set->add_layout(&layouts[9]);
}

inline void
init_prompt_layout(Layout_Set *set, Layout *layouts) {
    set->add_layout(&layouts[3]);
}

bool8 init_pipelines(Assets *assets) {


#if DEBUG
#if VULKAN
    //vulkan_print_allocated_descriptors(); // based on init_layouts
#endif // VULKAN
#endif // DEBUG

    Shader *shader;

    shader = find_shader(assets, SHADER_TEXT);
    shader->pipeline.depth_test = false;
    shader->vertex_info = get_vertex_xu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_text_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);

    shader = find_shader(assets, SHADER_COLOR);
    shader->pipeline.depth_test = false;
    shader->vertex_info = get_vertex_xu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_color_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);
        
    shader = find_shader(assets, SHADER_TEXTURE);
    shader->pipeline.depth_test = false;
    shader->vertex_info = get_vertex_xu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_texture_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);
    
    shader = find_shader(assets, SHADER_BASIC3D);
    shader->pipeline.depth_test = true;
    shader->vertex_info = get_vertex_xnu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_basic_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);

    shader = find_shader(assets, SHADER_COLOR3D);
    shader->pipeline.depth_test = true;
    shader->vertex_info = get_vertex_xnu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_color3D_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);
    
    shader = find_shader(assets, SHADER_TEXT3D);
    shader->pipeline.depth_test = true;
    shader->vertex_info = get_vertex_xnu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_text_frag_layout(shader, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);

    shader = find_shader(assets, SHADER_RAY);
    shader->pipeline.compute = true;
    init_ray_comp_layout(&shader->set, gfx.get_layouts());
    gfx.create_compute_pipeline(shader);

    shader = find_shader(assets, SHADER_PROMPT);
    shader->pipeline.depth_test = false;
    shader->vertex_info = get_vertex_xu_info();
    init_basic_vert_layout(&shader->set, gfx.get_layouts());
    init_prompt_layout(&shader->set, gfx.get_layouts());
    gfx.create_graphics_pipeline(shader);
        
    return false;
}
