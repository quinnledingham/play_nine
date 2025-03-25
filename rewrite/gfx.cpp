void gfx_init() {
  u32 layouts_count = 3;
  gfx.layouts_count = layouts_count;
  gfx.layouts = ARRAY_MALLOC(GFX_Layout, layouts_count);
  memset(gfx.layouts, 0, sizeof(GFX_Layout) * layouts_count);

  gfx.layouts[GFXID_SCENE].set_number = 0;
  gfx.layouts[GFXID_COLOR_2D].set_number = 1;
  gfx.layouts[GFXID_TEXTURE].set_number = 1;

  gfx.layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  gfx.layouts[GFXID_COLOR_2D].add_binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color.frag, text.frag
  gfx.layouts[GFXID_TEXTURE].add_binding(0, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 1, 0); // texture.frag

  for (u32 i = 0; i < layouts_count; i++) {
    vulkan_create_set_layout(&vk_ctx, &gfx.layouts[i]);
    vulkan_allocate_descriptor_set(&vk_ctx, &gfx.layouts[i]);
    vulkan_init_layout_offsets(&vk_ctx, &gfx.layouts[i]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_2D);
    shader->vertex_info = Vertex_XU::get_vertex_info();
    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_COLOR_2D]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_2D_TEXTURE);
    shader->vertex_info = Vertex_XU::get_vertex_info();
    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
  }
}

void gfx_bind_pipeline(u32 id) {
  gfx.active_shader_id = id;

  Pipeline *pipeline = find_pipeline(id);
  vulkan_bind_pipeline(pipeline);
}

Descriptor gfx_descriptor(u32 gfx_layout_id) {
  return vulkan_get_descriptor_set(&gfx.layouts[gfx_layout_id]);
}

inline void
gfx_default_viewport() {
  vulkan_set_viewport(gfx.window.dim.width, gfx.window.dim.height);
}

inline void
gfx_default_scissor() {
  vulkan_set_scissor(0, 0, gfx.window.dim.width, gfx.window.dim.height);
}
