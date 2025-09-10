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

inline void 
gfx_bind_compute_pipeline(u32 id) {
  gfx.active_shader_id = id;

  Pipeline *pipeline = find_pipeline(id);
  vulkan_bind_compute_pipeline(pipeline);
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

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
internal void
update_camera_with_mouse(Camera *camera, Vector2 delta_mouse, float32 x_speed, float32 y_speed) {   
    camera->pitch += (delta_mouse.x * x_speed) * DEG2RAD;
    camera->yaw += (delta_mouse.y * y_speed) * DEG2RAD;
    
    // doesnt break without this - just so it does not keep getting higher and higher
    float32 max_yaw = 2*PI;
    if (camera->pitch > max_yaw) camera->pitch = 0;
    if (camera->pitch < 0)       camera->pitch = max_yaw;

    // breaks with out this check
    float32 max_pitch = 89.0f * DEG2RAD;
    if (camera->yaw >  max_pitch) camera->yaw =  max_pitch;
    if (camera->yaw < -max_pitch) camera->yaw = -max_pitch;
}


internal void
set_camera_values(Camera *camera) {
  Vector3 up        = { 0, 1, 0 };
  Vector3 direction = { 1, 0, 0 };
  
  Vector3 orientation = camera->orientation;
  Quaternion x_rot = get_rotation(orientation.w, X_AXIS);
  Quaternion y_rot = get_rotation(-orientation.p, Y_AXIS);
  Quaternion z_rot = get_rotation(orientation.k, Z_AXIS);

  Quaternion rot = z_rot * y_rot * x_rot;

  camera->direction = normalized(rot * direction);
  camera->up = normalized(rot * up);
  camera->right = normalized(cross_product(camera->direction, camera->up));
  //camera->right = normalized(cross_product(camera->direction, up));
  //camera->up = normalized(cross_product(camera->right, camera->direction));
}

internal void
update_camera_with_keys(Camera *camera, Vector3 magnitude,
                        bool8 forward, bool8 backward,
                        bool8 left, bool8 right,
                        bool8 up, bool8 down) {
    if (forward)  camera->position   += camera->direction * magnitude;
    if (backward) camera->position   -= camera->direction * magnitude;
    if (right)    camera->position   += camera->right * magnitude;
    if (left)     camera->position   -= camera->right * magnitude;
    if (up)       camera->position   += camera->up * magnitude;
    if (down)     camera->position   -= camera->up * magnitude;
}

inline Matrix_4x4
get_camera_view(Camera *c) {
  Vector3 f = c->direction;
  Vector3 r = c->right;
  Vector3 u = c->up;

  Vector3 position = c->position;

  Vector3 t = { -dot_product(r, position), -dot_product(u, position), -dot_product(f, position) };

  Matrix_4x4 V = {
      r.x, u.x, f.x, 0,
      r.y, u.y, f.y, 0,
      r.z, u.z, f.z, 0,
      t.x, t.y, t.z, 1
  };

  //Matrix_4x4 X = intermediate_X();
  //Matrix_4x4 XV = multiply(X, V);
  return V;
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

internal float32
normalize_angle(float32 angle) {
    angle = fmodf(angle + PI, 2*PI);
    if (angle < 0) {
        angle += 2*PI;
    }
    return angle - PI;
}

internal Transform
linear_interpolate(Transform start, Transform end, float32 percent) {
    Transform result = {};
    result.x = linear_interpolate(start.x, end.x, percent);
    result.y = linear_interpolate(start.y, end.y, percent);
    result.z = linear_interpolate(start.z, end.z, percent);

    result.w = linear_interpolate(start.w, end.w, percent);
    result.p = linear_interpolate(start.p, end.p, percent);
    result.k = linear_interpolate(start.k, end.k, percent);

    return result;
}

internal Transform
slerp_intropolation(Transform start, Transform end, float32 percent) {
  Vector2 xz = spherical_interpolate(start.position.xz(), end.position.xz(), percent);

  Transform result = {};
  result.x = xz.x;
  result.z = xz.y;
  result.y = linear_interpolate(start.y, end.y, percent);

  result.w = linear_interpolate(start.w, end.w, percent);
  result.p = linear_interpolate(start.p, end.p, percent);
  result.k = linear_interpolate(start.k, end.k, percent);

  return result;
}

internal void normalize_angle_difference(float32 &start, const float32 end) {
  float32 diff = end - start;
  while (diff > PI) {
    start += 2*PI;
    diff = end - start;
  } 
  while (diff < -PI) {
    start -= 2*PI;
    diff = end - start;
  }
}

internal void
do_animation(Animation *a, float64 frame_time_s) {
  a->active = false;
  for (u32 i = 0; i < a->keyframes.size(); i++) {
    Animation_Keyframe *key = &a->keyframes[i];

    if (key->time_elapsed < key->time_duration) {
      a->active = true; // animation is still running
      key->time_elapsed += (float32)frame_time_s; // track how much of animation has played

      float32 percent = key->time_elapsed / key->time_duration; // percent completed for where to draw
      clamp(&percent, 0.0f, 1.0f);
      
      if (key->dynamic) {
        key->end = *key->dest;

        normalize_angle_difference(key->start.w, key->end.w);
        normalize_angle_difference(key->start.p, key->end.p);
        normalize_angle_difference(key->start.k, key->end.k);
      }

      switch(key->interpolation) {
        case INTERP_LERP:  *a->src = linear_interpolate(key->start, key->end, percent);  break;
        case INTERP_SLERP: *a->src = slerp_intropolation(key->start, key->end, percent); break;
        case INTERP_FUNC: {
          bool8 consume_keyframe = key->func(key->func_args); 
          if (consume_keyframe)
            key->time_elapsed = key->time_duration;
          else
            key->time_elapsed = 0.0f;
        } break;
      }

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
add_keyframe(Animation *a, Animation_Keyframe *new_key) {
  a->keyframes.insert(*new_key);
  u32 index = a->keyframes.insert_index;

  Animation_Keyframe *key = &a->keyframes[index - 1];
  Animation_Keyframe *previous_key = 0;
  if (index >= 2)
    previous_key = &a->keyframes[index - 2];

  if (previous_key && previous_key->interpolation != INTERP_FUNC) {
    key->start = previous_key->end;
  }

  if (key->dynamic) {
    key->end = *key->dest;
  }

  if (!key->time_duration) {
    key->time_duration = 1.0f; // defaults duration to one second
  }

  normalize_angle_difference(key->start.w, key->end.w);
  normalize_angle_difference(key->start.p, key->end.p);
  normalize_angle_difference(key->start.k, key->end.k);
};

internal void
add_keyframe(Animation *a, Transform &start, Transform &end, float32 time_duration) {
  Animation_Keyframe key = {};
  key.start = start;
  key.end = end;
  key.time_duration = time_duration;
  add_keyframe(a, &key);
}

internal void
add_keyframe(Animation *a, Transform &start, Transform &end, float32 time_duration, u32 interpolation) {
  Animation_Keyframe key = {};
  key.start = start;
  key.end = end;
  key.interpolation = interpolation;
  key.time_duration = time_duration;
  add_keyframe(a, &key);
}

internal void
add_dynamic_keyframe(Animation *a, Transform &start, Transform *dest, float32 time_duration) {
  Animation_Keyframe key = {};
  key.start = start;
  key.dynamic = true;
  key.dest = dest;
  key.time_duration = time_duration;
  add_keyframe(a, &key);
}

internal void
add_keyframe(Animation *a, bool8 (*func)(void *), void *args) {
  Animation_Keyframe key = {};
  key.interpolation = INTERP_FUNC;
  key.func = func;
  key.func_args = args;
  add_keyframe(a, &key);
}

internal Animation*
find_animation(Array<Animation> &animations, Transform *src) {
  // check for animation already on src
  for (u32 i = 0; i < animations.size(); i++) {
    Animation *a = &animations[i];
    // src Transform is already being animated
    if (a->active && a->src == src) {
      return a;
    }
  }

  // otherwise find open slot
  for (u32 i = 0; i < animations.size(); i++) {
    Animation *a = &animations[i];
    if (!a->active) {
      a->active = true;
      a->keyframes.clear();
      a->src = src;
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

internal u32
count_active_animations(Array<Animation> &animations) {
  u32 count = 0;
  for (u32 i = 0; i < animations.size(); i++) {
    Animation *a = &animations[i];
    if (a->active)
      count++;
  }
  return count;
}
