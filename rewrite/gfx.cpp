internal s32
gfx_start_frame() {
  if (gfx.window.minimized) {
    return GFX_SKIP_FRAME;
  }

  if (gfx.window.resized) {
    vulkan_destroy_frame_resources();
    vulkan_create_frame_resources();
    gfx.window.resized = false;
  }

  if (vulkan_start_frame()) {
    return GFX_ERROR;
  }

  // Resets all of the descriptor sets to be reallocated on next frame
  for (u32 i = 0; i < GFXID_COUNT; i++) {
      gfx.layouts[i].reset();
  }

  return GFX_DO_FRAME;
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
  return vulkan_get_descriptor(set, binding_index);
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

internal void
gfx_bind_descriptor_set(u32 gfx_id, void *data) {
  Descriptor_Set desc_set = gfx_descriptor_set(gfx_id);
  Descriptor desc = gfx_descriptor(&desc_set, 0);
  vulkan_update_ubo(desc, data);
  vulkan_bind_descriptor_set(desc_set);
}

internal void
gfx_ubo(u32 gfx_id, void *data, u32 binding) {
  Descriptor_Set desc_set = gfx_descriptor_set(gfx_id);
  Descriptor desc = gfx_descriptor(&desc_set, binding);
  vulkan_update_ubo(desc, data);
  vulkan_bind_descriptor_set(desc_set);
}



internal void
gfx_bind_bitmap(u32 gfx_id, Bitmap *bitmap, u32 binding) {
  Descriptor_Set texture_desc_set = gfx_descriptor_set(gfx_id);
  Descriptor texture_desc = gfx_descriptor(&texture_desc_set, binding);
  vulkan_set_bitmap(&texture_desc, bitmap);
  vulkan_bind_descriptor_set(texture_desc_set);
}


internal void
gfx_bind_bitmap(u32 gfx_id, u32 bitmap_id, u32 binding) {
  Bitmap *bitmap = find_bitmap(bitmap_id);
  gfx_bind_bitmap(gfx_id, bitmap, binding);
}

inline void
gfx_bind_atlas(Texture_Atlas *atlas) {
  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);
}

//
// Camera
//

inline Matrix_4x4 get_view(Camera camera)  { 
    return look_at(camera.position, camera.position + camera.target, camera.up); 
}

internal void
update_camera_target(Camera *camera) {
    Vector3 camera_direction = {
        cosf(DEG2RAD * (float32)camera->yaw) * cosf(DEG2RAD * (float32)camera->pitch),
        sinf(DEG2RAD * (float32)camera->pitch),
        sinf(DEG2RAD * (float32)camera->yaw) * cosf(DEG2RAD * (float32)camera->pitch)
    };
    camera->target = normalized(camera_direction);
}

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
internal void
update_camera_with_mouse(Camera *camera, Vector2 delta_mouse, float64 x_speed, float64 y_speed) {   
    camera->yaw   -= (float64)delta_mouse.x * x_speed;
    camera->pitch -= (float64)delta_mouse.y * y_speed;
    
    //print("%f\n", camera->yaw);

    // doesnt break withou this - just so it does not keep getting higher and higher
    float64 max_yaw = 360.0f;
    if (camera->yaw > max_yaw) camera->yaw = 0;
    if (camera->yaw < 0)       camera->yaw = max_yaw;

    // breaks with out this check
    float64 max_pitch = 89.0f;
    if (camera->pitch >  max_pitch) camera->pitch =  max_pitch;
    if (camera->pitch < -max_pitch) camera->pitch = -max_pitch;
}

internal void
update_camera_with_keys(Camera *camera, Vector3 magnitude,
                        bool8 forward, bool8 backward,
                        bool8 left, bool8 right,
                        bool8 up, bool8 down) {
    if (forward)  camera->position   += camera->target * magnitude;
    if (backward) camera->position   -= camera->target * magnitude;
    if (left)     camera->position   -= normalized(cross_product(camera->target, camera->up)) * magnitude;
    if (right)    camera->position   += normalized(cross_product(camera->target, camera->up)) * magnitude;
    if (up)       camera->position.y += magnitude.y;
    if (down)     camera->position.y -= magnitude.y;
}