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
update_camera_with_mouse(Camera *camera, Vector2 delta_mouse, float32 x_speed, float32 y_speed) {   
    camera->yaw   -= delta_mouse.x * x_speed;
    camera->pitch -= delta_mouse.y * y_speed;
    
    //print("%f\n", camera->yaw);

    // doesnt break withou this - just so it does not keep getting higher and higher
    float32 max_yaw = 360.0f;
    if (camera->yaw > max_yaw) camera->yaw = 0;
    if (camera->yaw < 0)       camera->yaw = max_yaw;

    // breaks with out this check
    float32 max_pitch = 89.0f;
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

//
// Animations
//

// computing animations

internal float32
linear_interpolate(float32 start, float32 end, float32 percent) {
    return start + ((end - start) * percent);
}

internal Vector2
spherical_interpolate(Vector2 start, Vector2 end, float32 percent) {
  // Compute magnitudes
  float32 start_mag = magnitude(start);
  float32 end_mag = magnitude(end);

  // Normalize input vectors to ensure they are on a sphere
  normalize(start);
  normalize(end);

  // Compute the dot product (cosine of the angle)
  float32 dot = start.x * end.x + start.y * end.y;

  // Clamp dot product to stay within valid range [-1, 1] (to avoid NaN issues)
  dot = fmaxf(-1.0f, fminf(1.0f, dot));

  // Compute the angle between start and end vectors
  float32 theta = acosf(dot);

  // If the angle is very small, linearly interpolate to avoid numerical instability
  if (theta < 1e-6) {
      return { 
          start.x + percent * (end.x - start.x), 
          start.y + percent * (end.y - start.y), 
      };
  }

  // Compute the interpolation weights
  float32 sinTheta = sinf(theta);
  float32 weightA = sinf((1.0f - percent) * theta) / sinTheta;
  float32 weightB = sinf(percent * theta) / sinTheta;

  // Compute SLERP result
  Vector2 result;
  result.x = weightA * start.x + weightB * end.x;
  result.y = weightA * start.y + weightB * end.y;

  // Interpolate the magnitude linearly and scale the result
  float32 interpolated_mag = start_mag + percent * (end_mag - start_mag);
  result = result * interpolated_mag;

  return result;
}

// Spherical linear interpolation (SLERP) between two unit vectors
internal Vector3 
spherical_interpolate(Vector3 start, Vector3 end, float32 percent) {
  // Compute magnitudes
  float32 start_mag = magnitude(start);
  float32 end_mag = magnitude(end);

  // Normalize input vectors to ensure they are on a sphere
  normalize(start);
  normalize(end);

  // Compute the dot product (cosine of the angle)
  float32 dot = start.x * end.x + start.y * end.y + start.z * end.z;

  // Clamp dot product to stay within valid range [-1, 1] (to avoid NaN issues)
  dot = fmaxf(-1.0f, fminf(1.0f, dot));

  // Compute the angle between start and end vectors
  float32 theta = acosf(dot);

  // If the angle is very small, linearly interpolate to avoid numerical instability
  if (theta < 1e-6) {
      return { 
          start.x + percent * (end.x - start.x), 
          start.y + percent * (end.y - start.y), 
          start.z + percent * (end.z - start.z) 
      };
  }

  // Compute the interpolation weights
  float32 sinTheta = sinf(theta);
  float32 weightA = sinf((1.0f - percent) * theta) / sinTheta;
  float32 weightB = sinf(percent * theta) / sinTheta;

  // Compute SLERP result
  Vector3 result;
  result.x = weightA * start.x + weightB * end.x;
  result.y = weightA * start.y + weightB * end.y;
  result.z = weightA * start.z + weightB * end.z;

  // Interpolate the magnitude linearly and scale the result
  float32 interpolated_mag = start_mag + percent * (end_mag - start_mag);
  result = result * interpolated_mag;

  return result;
}

internal Pose
linear_interpolate(Pose start, Pose end, float32 percent) {
    Pose result = {};
    result.x = linear_interpolate(start.x, end.x, percent);
    result.y = linear_interpolate(start.y, end.y, percent);
    result.z = linear_interpolate(start.z, end.z, percent);

    result.w = linear_interpolate(start.w, end.w, percent);
    result.p = linear_interpolate(start.p, end.p, percent);
    result.k = linear_interpolate(start.k, end.k, percent);

    return result;
}

internal Pose
slerp_intropolation(Pose start, Pose end, float32 percent) {
  Vector2 xz = spherical_interpolate(start.position.xz(), end.position.xz(), percent);

  Pose result = {};
  result.x = xz.x;
  result.z = xz.y;
  result.y = linear_interpolate(start.y, end.y, percent);

  result.w = linear_interpolate(start.w, end.w, percent);
  result.p = linear_interpolate(start.p, end.p, percent);
  result.k = linear_interpolate(start.k, end.k, percent);

  return result;
}

internal void
do_animation(Animation *a, float64 frame_time_s) {
  a->active = false;
  for (u32 i = 0; i < a->keyframes.size(); i++) {
    Animation_Keyframe *key = &a->keyframes[i];

    if (key->time_elapsed < key->time_duration) {
      a->active = true;
      key->time_elapsed += (float32)frame_time_s;

      float32 percent = key->time_elapsed / key->time_duration;
      *a->src = slerp_intropolation(key->start, key->end, percent);
      break;
    }
  }
}

// setting up animations

internal void
do_animations(Array<Animation> &animations, float64 frame_time_s) {
    for (u32 i = 0; i < animations.size(); i++) {
        Animation *a = &animations[i];
        if (a->active) {
            do_animation(a, frame_time_s);
        }
    }
}

internal void 
normalize_angle_difference(float32 &start, const float32 end) {
  if (end - start > 180.0f) {
    start += 360.0f;  
  } else if (end - start < -180.0f) {
    start -= 360.0f;
  }
}

internal void
add_keyframe(Animation *a, Animation_Keyframe *new_key) {
  a->keyframes.insert(*new_key);
  u32 index = a->keyframes.insert_index;

  Animation_Keyframe *key = &a->keyframes[index - 1];
  Animation_Keyframe *previous_key = 0;
  if (index >= 2)
    previous_key = &a->keyframes[index - 2];

  if (previous_key) {
    key->start = previous_key->end;
  }

  normalize_angle_difference(key->start.w, key->end.w);
  normalize_angle_difference(key->start.p, key->end.p);
  normalize_angle_difference(key->start.k, key->end.k);
};

internal Animation*
find_animation(Array<Animation> &animations, Pose *src) {
  for (u32 i = 0; i < animations.size(); i++) {
    Animation *a = &animations[i];
    if (!a->active) {
      a->active = true;
      a->keyframes.clear();
      a->src = src;
      return a;
    }
    // src pose is already being animated
    if (a->active && a->src == src) {
      return a;
    }
  }
  ASSERT(0);
  return 0;
}

internal void
init_animations(Array<Animation> &animations) {
    animations.init(10);
    animations.clear();
    for (u32 i = 0; i < animations.size(); i++) {
        Animation *a = &animations[i];
        a->keyframes.init(10);
        a->keyframes.clear();
    }
};