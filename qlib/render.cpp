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

void GFX::update_resolution() {
    resolution = get_resolution(window_dim, resolution_scale);

    if (resolution == window_dim) {
        resolution_scaling = FALSE;
    } else {
        resolution_scaling = TRUE;
    }
}

void GFX::scissor_push(Vector2 coords, Vector2 dim) {
    Vector2 factor = {
        (float32)resolution.x / (float32)window_dim.x,
        (float32)resolution.y / (float32)window_dim.y
    };
    
    Rect *rect = &scissor_stack[scissor_stack_index++];
    rect->coords = coords * factor;
    rect->dim = dim * factor;
    render_set_scissor((s32)rect->coords.x, (s32)rect->coords.y, (s32)rect->dim.x, (s32)rect->dim.y);
}

void GFX::scissor_pop() {
    scissor_stack_index--;
    Rect rect = scissor_stack[scissor_stack_index - 1];
    render_set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (u32)rect.dim.x, (u32)rect.dim.y);
}
