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
update_camera_with_mouse(Camera *camera, Vector2_s32 delta_mouse, float64 x_speed, float64 y_speed) {   
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
update_camera_with_keys(Camera *camera, Vector3 target, Vector3 up_v, Vector3 magnitude,
                        bool8 forward, bool8 backward,
                        bool8 left, bool8 right,
                        bool8 up, bool8 down) {
    if (forward)  camera->position   += target * magnitude;
    if (backward) camera->position   -= target * magnitude;
    if (left)     camera->position   -= normalized(cross_product(target, up_v)) * magnitude;
    if (right)    camera->position   += normalized(cross_product(target, up_v)) * magnitude;
    if (up)       camera->position.y += magnitude.y;
    if (down)     camera->position.y -= magnitude.y;
}

// API3D

//
// GFX
//

internal float32 
get_resolution_scale(u32 resolution_mode) {
    float32 resolution_scales[4] = {
        0.25f,
        0.5f,
        0.75f,
        1.0f
    };

    return resolution_scales[resolution_mode];
}

internal Vector2_s32
get_resolution(Vector2_s32 in_resolution, float32 scale) {
    Vector2 new_resolution = cv2(in_resolution) * scale;
    Vector2_s32 out_resolution = { s32(new_resolution.x), s32(new_resolution.y) };
    return out_resolution;
}

Vector2_s32 GFX::get_resolution(float32 scale) {
    Vector2 new_resolution = cv2(window_dim) * scale;
    Vector2_s32 out_resolution = { s32(new_resolution.x), s32(new_resolution.y) };
    return out_resolution;
}

void GFX::set_resolution(Vector2_s32 new_resolution) {
    resolution = new_resolution;
}

void GFX::update_resolution() {
    resolution = get_resolution(resolution_scale);

    if (resolution == window_dim) {
        resolution_scaling = FALSE;
    } else {
        resolution_scaling = TRUE;
    }
}

void GFX::set_window_dim(Vector2_s32 dim) {
    window_dim = dim;
}


void GFX::scissor_push(Vector2 coords, Vector2 dim) {
    Vector2 factor = {
        (float32)resolution.x / (float32)window_dim.x,
        (float32)resolution.y / (float32)window_dim.y
    };
    
    Rect *rect = &scissor_stack[scissor_stack_index++];
    rect->coords = coords * factor;
    rect->dim = dim * factor;
    set_scissor((s32)rect->coords.x, (s32)rect->coords.y, (s32)rect->dim.x, (s32)rect->dim.y);
}

void GFX::scissor_pop() {
    scissor_stack_index--;
    Rect rect = scissor_stack[scissor_stack_index - 1];
    set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (u32)rect.dim.x, (u32)rect.dim.y);
}

u8 GFX::get_flags() {
    u8 flags = 0;
    if (vsync)
        flags |= GFX_VSYNC;
    if (anti_aliasing)
        flags |= GFX_ANTI_ALIASING;
    return flags;
}

Descriptor GFX::descriptor_set(u32 layout_id) {
    if (!vulkan_info.recording_frame) {
        // if we are not in the middle of recording commands for a frame, don't check active
        // shader for the layout.
        return get_descriptor_set(&layouts[layout_id]);
    }

    Shader *shader = &assets->data[vulkan_info.active_shader_id].shader;
    Layout_Set *set = &shader->set;
    
    for (u32 i = 0; i < set->layouts_count; i++) {
        if (set->layouts[i]->id == layout_id) {
            return get_descriptor_set(set->layouts[i]);
        }
    }
        
    logprint("gfx_get_descriptor_set()", "no layout in set with id in shader: %d\n", vulkan_info.active_shader_id);
    ASSERT(0);
    return {};
}

Descriptor GFX::descriptor_set_index(u32 layout_id, u32 return_index) {
    Layout *layout = &layouts[layout_id];
    
    if (layout->sets_in_use + 1 > layout->max_sets)
        ASSERT(0);

    if (return_index < layout->sets_in_use) {
        logprint("gfx_get_descriptor_set()", "descriptor could already be in use\n");
    } else {
        layout->sets_in_use = return_index + 1; // jump to after this set
    }

    Descriptor desc = {};
    desc.binding = layout->bindings[0];
    desc.offset = layout->offsets[return_index];
    desc.set_number = layout->set_number;
    desc.vulkan_set = &layout->descriptor_sets[return_index];

    return desc;
}

void GFX::create_swap_chain() {
    recreate_swap_chain(&vulkan_info, window_dim, resolution, get_flags(), assets);
}