//
// Camera
//

internal void
update_camera_target(Camera *camera) {
    Vector3 camera_direction = {
        cosf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch)
    };
    camera->target = normalized(camera_direction);
}

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
internal void
update_camera_with_mouse(Camera *camera, Vector2_s32 delta_mouse, Vector2 move_speed) {    
    camera->yaw   -= (float32)delta_mouse.x * move_speed.x;
    camera->pitch -= (float32)delta_mouse.y * move_speed.y;
    
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
// Model
//

void render_init_model(Model *model) {
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
        Mesh *mesh = &model->meshes[mesh_index];
        render_init_mesh(mesh);
        render_create_texture(&mesh->material.diffuse_map);
    }
}

void render_draw_model(Model *model, Shader *shader, Vector3 position, Quaternion rotation) {
    for (u32 i = 0; i < model->meshes_count; i++) {

        Matrix_4x4 model_matrix = create_transform_m4x4(position, rotation, {1.0f, 1.0f, 1.0f});
        render_push_constants(&shader->layout_sets[2], (void *)&model_matrix);

        if (model->meshes[i].material.diffuse_map.memory != 0) {
            Descriptor_Set *object_set = render_get_descriptor_set(shader, 1);
            render_set_bitmap(object_set, &model->meshes[i].material.diffuse_map, 1);
            render_bind_descriptor_set(object_set, 1);
        }

        render_draw_mesh(&model->meshes[i]);
    }
}