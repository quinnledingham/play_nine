//
// Camera
//

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
// Model
//

void render_draw_model(Model *model, Shader *shader, Vector3 position, Quaternion rotation) {
    Vector4 color = { 148, 99, 46, 1 };
    Object object = {};

    Descriptor bitmap_desc = render_get_descriptor_set(&layouts[2]);
    Descriptor color_set = render_get_descriptor_set(&layouts[5]);
    
    for (u32 i = 0; i < model->meshes_count; i++) {
        if (model->meshes[i].material.diffuse_map.memory != 0) {
            object.index = render_set_bitmap(&bitmap_desc, &model->meshes[i].material.diffuse_map);
            render_bind_descriptor_set(bitmap_desc);
        } else {
            render_update_ubo(color_set, &color);
            render_bind_descriptor_set(color_set);
        }

        object.model = create_transform_m4x4(position, rotation, {15.0f, 1.0f, 15.0f});
        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

        render_draw_mesh(&model->meshes[i]);
    }
}

Descriptor texture_desc = {};

void render_draw_model(Model *model, Descriptor color_set, s32 front_index, s32 back_index, Matrix_4x4 model_matrix, bool8 flipped) {
    u32 draw[3] = { 1, 0, 2 };
    u32 flipped_draw[3] = { 2, 0, 1 };

    for (u32 i = 0; i < model->meshes_count; i++) {
        u32 draw_index = draw[i];
        if (flipped)
            draw_index = flipped_draw[i];

        Object object = {};
        object.model = model_matrix;
        object.index = back_index;

        switch(draw_index) {
            case 0: 
                render_bind_pipeline(&color_pipeline); 
                render_bind_descriptor_set(color_set);
            break;

            case 1:
                object.index = front_index;
            case 2: 
                render_bind_pipeline(&basic_pipeline); 
                render_bind_descriptor_set(texture_desc);
            break;
        }

        render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
        render_draw_mesh(&model->meshes[draw_index]);
    }
}

void render_draw_model(Model *model, Render_Pipeline *color_pipeline, Vector4 color, Matrix_4x4 model_matrix) {
    render_bind_pipeline(color_pipeline);
    render_bind_descriptor_set(light_set_2);

    Descriptor color_set = render_get_descriptor_set(&layouts[5]);
    render_update_ubo(color_set, (void *)&color);
    render_bind_descriptor_set(color_set);

    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model_matrix, sizeof(Matrix_4x4));

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}