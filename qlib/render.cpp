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
    Descriptor_Set *bitmap_set = render_get_descriptor_set(shader, 3);

    Vector4 color = { 148, 99, 46, 1 };
    Descriptor_Set *color_set = render_get_descriptor_set(shader, 3);
    render_update_ubo(color_set, 0, (void*)&color, false);

    for (u32 i = 0; i < model->meshes_count; i++) {
        Matrix_4x4 model_matrix = create_transform_m4x4(position, rotation, {15.0f, 1.0f, 15.0f});
        render_push_constants(&shader->layout_sets[2], (void *)&model_matrix);

        if (model->meshes[i].material.diffuse_map.memory != 0) {
            render_set_bitmap(bitmap_set, &model->meshes[i].material.diffuse_map, 2);
            render_bind_descriptor_set(bitmap_set, 2);
        } else {
            
            render_bind_descriptor_set(color_set, 2);
        }

        render_draw_mesh(&model->meshes[i]);
    }
}

Descriptor_Set *texture_set = {};

void render_draw_model(Model *model, Descriptor_Set *color_set, s32 front_index, s32 back_index, Matrix_4x4 model_matrix) {
    //Descriptor_Set *bitmap_set = render_get_descriptor_set(basic_pipeline.shader, 3);
    //render_set_bitmap(bitmap_set, bitmap, 2);

    for (u32 i = 0; i < model->meshes_count; i++) {
        Shader *shader = 0;
        if (i == 0) {
            render_bind_pipeline(&color_pipeline);
            shader = color_pipeline.shader;
        } else if (i == 1 || i == 2) {
            render_bind_pipeline(&basic_pipeline);
            shader = basic_pipeline.shader;
        }

        Object object = {};
        object.model = model_matrix;

        if (i == 0) {
            render_bind_descriptor_set(color_set, 2);
            //render_bind_descriptor_set(light_set, 1);
        } else if (i == 1) {
            //render_bind_descriptor_set(front_set, 2);
            render_bind_descriptor_set(texture_set, 2);
            object.index = front_index;
        } else if (i == 2) {
            //render_bind_descriptor_set(back_set, 2);
            render_bind_descriptor_set(texture_set, 2);
            object.index = back_index;
        }

        render_push_constants(&shader->layout_sets[2], (void *)&object);

        render_draw_mesh(&model->meshes[i]);
    }
}

void render_draw_model(Model *model, Render_Pipeline *color_pipeline, Vector4 color, Matrix_4x4 model_matrix) {
    Shader *shader = 0;
    render_bind_pipeline(color_pipeline);
    shader = color_pipeline->shader;
    render_bind_descriptor_set(light_set_2, 1);

    Descriptor_Set *object_set = render_get_descriptor_set(shader, 3);
    render_update_ubo(object_set, 0, (void*)&color, false);
    render_bind_descriptor_set(object_set, 2);

    render_push_constants(&shader->layout_sets[2], (void *)&model_matrix);

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}