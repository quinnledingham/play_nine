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
