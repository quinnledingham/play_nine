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

    Descriptor bitmap_desc = vulkan_get_descriptor_set(&layouts[2]);
    VkDescriptorSet color_set = vulkan_get_descriptor_set2(&layouts[5]);
    
    for (u32 i = 0; i < model->meshes_count; i++) {
        if (model->meshes[i].material.diffuse_map.memory != 0) {
            object.index = vulkan_set_bitmap(&bitmap_desc, &model->meshes[i].material.diffuse_map);
            vulkan_bind_descriptor_set(bitmap_desc);
        } else {
            vulkan_bind_descriptor_set(color_set, 2, (void*)&color, sizeof(Vector4));
        }

        object.model = create_transform_m4x4(position, rotation, {15.0f, 1.0f, 15.0f});
        vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

        render_draw_mesh(&model->meshes[i]);
    }
}

Descriptor texture_desc = {};

void render_draw_model(Model *model, VkDescriptorSet color_set, Vector4 color, s32 front_index, s32 back_index, Matrix_4x4 model_matrix) {
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
            vulkan_bind_descriptor_set(color_set, 2, (void*)&color, sizeof(Vector4));
        } else if (i == 1) {
            vulkan_bind_descriptor_set(texture_desc);
            object.index = front_index;
        } else if (i == 2) {
            vulkan_bind_descriptor_set(texture_desc);
            object.index = back_index;
        }

        vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

        render_draw_mesh(&model->meshes[i]);
    }
}

void render_draw_model(Model *model, Render_Pipeline *color_pipeline, Vector4 color, Matrix_4x4 model_matrix) {
    Shader *shader = 0;
    render_bind_pipeline(color_pipeline);
    shader = color_pipeline->shader;
    vulkan_bind_descriptor_set(light_set_2);

    VkDescriptorSet color_set = vulkan_get_descriptor_set2(&layouts[5]);
    vulkan_bind_descriptor_set(color_set, 2, (void*)&color, sizeof(Vector4));

    vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&model_matrix, sizeof(Matrix_4x4));

    for (u32 i = 0; i < model->meshes_count; i++) {
        render_draw_mesh(&model->meshes[i]);
    }
}