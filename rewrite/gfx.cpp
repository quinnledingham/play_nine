void GFX::init() {
  u32 layouts_count = 2;
  layouts = ARRAY_MALLOC(GFX_Layout, layouts_count);
  memset(layouts, 0, sizeof(GFX_Layout) * layouts_count);

  layouts[GFXID_SCENE].set_number = 0;
  layouts[GFXID_COLOR_2D].set_number = 1;

  layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  layouts[GFXID_COLOR_2D].add_binding(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Vector4)); // color.frag, text.frag

  for (u32 i = 0; i < layouts_count; i++) {
    vulkan_create_set_layout(vk_ctx, &layouts[i]);
    vulkan_allocate_descriptor_set(vk_ctx, &layouts[i]);
    vulkan_init_layout_offsets(vk_ctx, &layouts[i]);
  }

  Pipeline *shader = find_pipeline(PIPELINE_2D);
  shader->vertex_info = Vertex_XU::get_vertex_info();
  shader->set.add_layout(&layouts[GFXID_SCENE]);
  shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

  shader->set.add_layout(&layouts[GFXID_COLOR_2D]);
}

void GFX::bind_pipeline(u32 id) {
  active_shader_id = id;

  Pipeline *pipeline = find_pipeline(id);
  vulkan_bind_pipeline(vk_ctx, pipeline);
}

Descriptor GFX::descriptor(u32 gfx_layout_id) {
  return vulkan_get_descriptor_set(gfx.vk_ctx, &layouts[gfx_layout_id]);
}

void GFX::default_viewport() {
  vulkan_set_viewport(vk_ctx, window.dim.width, window.dim.height);
}

void GFX::default_scissor() {
  vulkan_set_scissor(vk_ctx, 0, 0, window.dim.width, window.dim.height);
}