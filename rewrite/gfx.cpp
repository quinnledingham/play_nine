void gfx_init() {
  u32 layouts_count = GFXID_COUNT;
  gfx.layouts_count = layouts_count;
  gfx.layouts = ARRAY_MALLOC(GFX_Layout, layouts_count);
  memset(gfx.layouts, 0, sizeof(GFX_Layout) * layouts_count);

  gfx.layouts[GFXID_SCENE].set_number = 0;
  gfx.layouts[GFXID_LOCAL].set_number = 1;
  //gfx.layouts[GFXID_TEXTURE].set_number = 1;

  gfx.layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  gfx.layouts[GFXID_LOCAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Local)); // color.frag, text.frag
  gfx.layouts[GFXID_LOCAL].add_binding(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 1, 0); // texture.frag

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

    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
  }
}

inline void 
gfx_bind_pipeline(u32 id) {
  gfx.active_shader_id = id;

  Pipeline *pipeline = find_pipeline(id);
  vulkan_bind_pipeline(pipeline);
}

inline Descriptor_Set 
gfx_descriptor_set(u32 gfx_layout_id) {
  return vulkan_get_descriptor_set(&gfx.layouts[gfx_layout_id]);
}

inline Descriptor 
gfx_descriptor(Descriptor_Set *set, u32 binding_index) {
  return vulkan_get_descriptor_set(set, binding_index);
}

inline void
gfx_default_viewport() {
  vulkan_set_viewport(gfx.window.dim.width, gfx.window.dim.height);
}

inline void
gfx_default_scissor() {
  vulkan_set_scissor(0, 0, gfx.window.dim.width, gfx.window.dim.height);
}

internal void
gfx_scissor_push(Vector2 coords, Vector2 dim) {
  Vector2 factor = {
      (float32)gfx.window.resolution.x / (float32)gfx.window.dim.x,
      (float32)gfx.window.resolution.y / (float32)gfx.window.dim.y
  };
  
  Rect rect = {};
  rect.coords = coords * factor;
  rect.dim = dim * factor;
  gfx.scissor_stack.push(rect);

  vulkan_set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (s32)rect.dim.x, (s32)rect.dim.y);
}

internal void
gfx_scissor_pop() {
  gfx.scissor_stack.pop();
  if (gfx.scissor_stack.index > 0) {
    Rect rect = gfx.scissor_stack.top();
    vulkan_set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (s32)rect.dim.x, (s32)rect.dim.y);
  }
}
