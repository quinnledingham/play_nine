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

#define GFX_FUNC(n) n = &API3D_EXT(n)

bool8 GFX::sdl_init(SDL_Window *window) {
    GFX_FUNC(recreate_swap_chain);

    GFX_FUNC(set_scissor);
    GFX_FUNC(set_viewport);
    GFX_FUNC(depth_test);
    GFX_FUNC(clear_color);
    //GFX_FUNC(start_frame);
    GFX_FUNC(end_frame);
    GFX_FUNC(start_compute);
    GFX_FUNC(end_compute);
    GFX_FUNC(dispatch);

    GFX_FUNC(get_descriptor_set);
    GFX_FUNC(bind_descriptor_set);
    GFX_FUNC(bind_descriptor_sets);
    GFX_FUNC(create_set_layout);
    GFX_FUNC(allocate_descriptor_set);
    GFX_FUNC(init_layout_offsets);

    GFX_FUNC(create_texture);
    GFX_FUNC(destroy_texture);
    GFX_FUNC(set_texture);
    GFX_FUNC(delete_texture);
    GFX_FUNC(set_bitmap);

    GFX_FUNC(immediate_vertex_xnu);
    GFX_FUNC(immediate_vertex_xu);
    GFX_FUNC(draw_immediate);

    GFX_FUNC(push_constants);
    GFX_FUNC(bind_shader);
    GFX_FUNC(create_graphics_pipeline);
    GFX_FUNC(create_compute_pipeline);

    GFX_FUNC(init_mesh);
    GFX_FUNC(draw_mesh);
    GFX_FUNC(update_ubo);

    GFX_FUNC(create_buffer);
    GFX_FUNC(set_storage_buffer);

    GFX_FUNC(assets_cleanup);
    GFX_FUNC(cleanup_layouts);
    GFX_FUNC(wait_frame);
    GFX_FUNC(cleanup);


    if (vulkan_sdl_init(window))
        return 1;

    return 0;
}

bool8 GFX::start_frame() {
    if (resized) {
        create_swap_chain();
        resized = false;
    }

    if (vulkan_start_frame())
        return 0;

    set_viewport(resolution.width, resolution.height);
    scissor_stack_index = 0;
    scissor_push({ 0, 0 }, cv2(window_dim));

    return 0;
}

//
// Draw
//

global Descriptor shapes_color_descriptor;

internal void
draw_shape(Shape shape) {
    Object object = {};

    switch(shape.draw_type) {
        case Shape_Draw_Type::COLOR: {
            gfx.bind_shader(SHADER_COLOR);
            gfx.bind_descriptor_sets(shapes_color_descriptor, &shape.color);
        } break;

        case Shape_Draw_Type::TEXTURE: {
            gfx.bind_shader(SHADER_TEXTURE);

            Descriptor desc = gfx.descriptor_set(3);
            object.index = gfx.set_bitmap(&desc, shape.bitmap);
            gfx.bind_descriptor_set(desc);
        } break;

        case Shape_Draw_Type::TEXT: {
            
        } break;
    }

    if (shape.type == SHAPE_RECT || shape.type == SHAPE_CIRCLE) {
        shape.coords.x += shape.dim.x / 2.0f;
        shape.coords.y += shape.dim.y / 2.0f; // coords = top left corner
    }

    object.model = create_transform_m4x4(shape.coords, shape.rotation, shape.dim);
    gfx.push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    switch(shape.type) {
        case SHAPE_RECT:   gfx.draw_mesh(&gfx.rect_mesh);   break;
        case SHAPE_CIRCLE: gfx.draw_mesh(&gfx.circle_mesh); break;
        //case SHAPE_SPHERE: gfx.draw_mesh(&shapes.sphere_mesh); break;
        default: logprint("draw_shape()", "not a valid shape type\n");
    }
}

// Rect

internal void
init_rect_indices(u32 *indices, 
                  u32 top_left, u32 top_right,
                  u32 bottom_left, u32 bottom_right) {
    indices[0] = top_left;
    indices[1] = bottom_right;
    indices[2] = bottom_left;
    indices[3] = top_left;
    indices[4] = top_right;
    indices[5] = bottom_right;
}

internal Mesh
get_rect_mesh() {
    Mesh mesh = {};
    mesh.vertices_count = 4;

    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
    Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;

    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
    /*
    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, -1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, -1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, -1}, {1, 0} };
    vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, -1}, {1, 1} };
    */
    mesh.indices_count = 6;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    // vertex locations
    u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
    /* 
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_left;
    mesh.indices[2] = bottom_right;
    mesh.indices[3] = top_left;
    mesh.indices[4] = bottom_right;
    mesh.indices[5] = top_right;
    */

    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_right;
    mesh.indices[2] = bottom_left;
    mesh.indices[3] = top_left;
    mesh.indices[4] = top_right;
    mesh.indices[5] = bottom_right;

    mesh.vertex_info = get_vertex_xnu_info();
    gfx.init_mesh(&mesh);

    return mesh;
}

internal Mesh
get_rect_mesh_2D() {
    Mesh mesh = {};
    mesh.vertices_count = 4;
    
    mesh.vertices = ARRAY_MALLOC(Vertex_XU, mesh.vertices_count);
    Vertex_XU *vertices = (Vertex_XU *)mesh.vertices;

    vertices[0] = Vertex_XU{ {-0.5, -0.5 }, {0, 0} };
    vertices[1] = Vertex_XU{ {-0.5,  0.5 }, {0, 1} };
    vertices[2] = Vertex_XU{ { 0.5, -0.5 }, {1, 0} };
    vertices[3] = Vertex_XU{ { 0.5,  0.5 }, {1, 1} };

    mesh.indices_count = 6;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    // vertex locations
    u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
   
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_right;
    mesh.indices[2] = bottom_left;
    mesh.indices[3] = top_left;
    mesh.indices[4] = top_right;
    mesh.indices[5] = bottom_right;

    mesh.vertex_info = get_vertex_xu_info();
    gfx.init_mesh(&mesh);

    return mesh;
}

void GFX::draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Vector4 color) {
    Vector3 coords_v3 = { coords.x, coords.y, 0 };
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    Vector3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_RECT;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = Shape_Draw_Type::COLOR;
    shape.color = color;
    draw_shape(shape);
}

void GFX::draw_rect(Rect rect, Vector4 color) {
    draw_rect(rect.coords, rect.rotation, rect.dim, color);
};

void GFX::draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Bitmap *bitmap) {
    Vector3 coords_v3 = { coords.x, coords.y, 0 };
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    Vector3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_RECT;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = Shape_Draw_Type::TEXTURE;
    shape.bitmap = bitmap;
    draw_shape(shape);
}

// Triangle

internal Mesh
init_triangle_mesh() {
    Mesh mesh = {};
    mesh.vertices_count = 3;
    
    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
    Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;

    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    
    mesh.indices_count = 3;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    mesh.indices[0] = 1;
    mesh.indices[1] = 0;
    mesh.indices[2] = 2;

    mesh.vertex_info = get_vertex_xnu_info();
    gfx.init_mesh(&mesh);

    return mesh;
}
 
void GFX::draw_triangle(Vector3 coords, Vector3 rotation, Vector3 dim, Vector4 color) {
    Quaternion rotation_quat_x = get_rotation(rotation.x, { 1, 0, 0 });
    Quaternion rotation_quat_y = get_rotation(rotation.y, { 0, 1, 0 });
    Quaternion rotation_quat_z = get_rotation(rotation.z, { 0, 0, 1 });
    Quaternion rotation_quat = rotation_quat_x * rotation_quat_y * rotation_quat_z;

    //Shader *shader = find_shader(global_assets, "COLOR3D");
    //render_bind_pipeline(&shader->pipeline);
    bind_shader(SHADER_COLOR3D);
    bind_descriptor_set(light_set);
    
    Descriptor color_desc = descriptor_set(5);
    update_ubo(color_desc, (void *)&color);
    bind_descriptor_set(color_desc);

    Object object = {};
    object.model = create_transform_m4x4(coords, rotation_quat, dim);
    push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));  

    draw_mesh(&triangle_3D_mesh);    
}

// Circle

internal void
init_circle_mesh(Mesh *mesh) {
    u32 circle_vertices = 64;
    float32 deg_between_vertices = 360.0f / circle_vertices;
    float32 radius = 0.5f;
    
    // allocating memory
    mesh->vertices_count = 1 + circle_vertices; // +1 for center
    mesh->vertices = ARRAY_MALLOC(Vertex_XU, mesh->vertices_count);
    Vertex_XU *vertices = (Vertex_XU *)mesh->vertices;

    mesh->indices_count = circle_vertices * 3 + 6;
    mesh->indices = ARRAY_MALLOC(u32, mesh->indices_count);
    
    // center vertex
    vertices[0] = Vertex_XU{ { 0, 0 }, { 0.5, 0.5 } };
    
    u32 indices_index = 0;
    for (u32 i = 0; i < circle_vertices; i++)
    {
        Vector2 coords = {};
        Vector2 texture_coords = {};
        float32 rad = DEG2RAD * (i * deg_between_vertices);
        
        coords.x = radius * cosf(rad);
        coords.y = radius * sinf(rad);
        
        // @WARNING NOT CORRECT
        texture_coords.x = coords.x;
        texture_coords.y = coords.y;
        
        vertices[i + 1] = Vertex_XU{ coords, texture_coords };
        
        if (i == 0) continue;
        
        // Make triangles
        mesh->indices[indices_index++] = 0;
        mesh->indices[indices_index++] = i - 1;
        mesh->indices[indices_index++] = i;
    }
    
    mesh->indices[indices_index++] = 0;
    mesh->indices[indices_index++] = circle_vertices - 1;
    mesh->indices[indices_index++] = circle_vertices;
    
    mesh->indices[indices_index++] = 0;
    mesh->indices[indices_index++] = circle_vertices;
    mesh->indices[indices_index++] = 1;
    
    mesh->vertex_info = get_vertex_xu_info();
    gfx.init_mesh(mesh);
}

void GFX::draw_circle(Vector2 coords, float32 rotation, float32 radius, Vector4 color) {
    Vector3 coords_v3 = { coords.x, coords.y, 0 };
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    Vector3 dim_v3 = { radius, radius, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_CIRCLE;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = Shape_Draw_Type::COLOR;
    shape.color = color;
    draw_shape(shape);
}

// Cube

internal Mesh
get_cube_mesh(bool32 out) {
    Mesh mesh = {};
    
    mesh.vertices_count = 8;
    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
    Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;
    
    // back
    vertices[0] = { {-0.5, -0.5, -0.5}, { 1,  1, 1}, {0, 0} }; // bottom left
    vertices[1] = { {-0.5,  0.5, -0.5}, { 1, -1, 1}, {0, 1} }; // top left
    vertices[2] = { { 0.5, -0.5, -0.5}, {-1,  1, 1}, {1, 0} }; // bottom right
    vertices[3] = { { 0.5,  0.5, -0.5}, {-1, -1, 1}, {1, 1} }; // top right
    
    // forward
    vertices[4] = { {-0.5, -0.5, 0.5}, { 1,  1, -1}, {0, 0} }; // bottom left
    vertices[5] = { {-0.5,  0.5, 0.5}, { 1, -1, -1}, {0, 1} }; // top left
    vertices[6] = { { 0.5, -0.5, 0.5}, {-1,  1, -1}, {1, 0} }; // bottom right
    vertices[7] = { { 0.5,  0.5, 0.5}, {-1, -1, -1}, {1, 1} }; // top right

    mesh.indices_count = 6 * 6; // 6 indices per side (rects), 6 sides
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
    
    if (out) {
        init_rect_indices(mesh.indices + 0,  3, 1, 2, 0); // back
        init_rect_indices(mesh.indices + 6,  5, 7, 4, 6); // front
        init_rect_indices(mesh.indices + 12, 1, 3, 5, 7); // top
        init_rect_indices(mesh.indices + 18, 4, 6, 0, 2); // bottom
        init_rect_indices(mesh.indices + 24, 1, 5, 0, 4); // left
        init_rect_indices(mesh.indices + 30, 7, 3, 6, 2); // right
    } else {
        init_rect_indices(mesh.indices + 0,  1, 3, 0, 2); // back
        init_rect_indices(mesh.indices + 6,  7, 5, 6, 4); // front
        init_rect_indices(mesh.indices + 12, 5, 7, 1, 3); // top
        init_rect_indices(mesh.indices + 18, 0, 2, 4, 6); // bottom
        init_rect_indices(mesh.indices + 24, 5, 1, 4, 0); // left
        init_rect_indices(mesh.indices + 30, 3, 7, 2, 6); // right
    }

    mesh.vertex_info = get_vertex_xnu_info();
    gfx.init_mesh(&mesh);
    
    return mesh;
}
internal Mesh get_cube_mesh() { return get_cube_mesh(true); }

void GFX::draw_cube(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
/*
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    
    Shape shape = {};
    shape.type = SHAPE_SPHERE;
    shape.coords = coords;
    shape.rotation = rotation_quat;
    shape.dim = dim;
    shape.draw_type = Shape_Draw_Type::COLOR;
    shape.color = color;
    draw_shape(shape);
*/

    bind_shader(SHADER_COLOR3D);
    bind_descriptor_set(light_set);

    Descriptor object_set = descriptor_set(5);
    update_ubo(object_set, (void *)&color);
    bind_descriptor_set(object_set);

    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    Matrix_4x4 model = create_transform_m4x4(coords, rotation_quat, dim);
    push_constants(SHADER_STAGE_VERTEX, (void *)&model, sizeof(Matrix_4x4)); 

    draw_mesh(&cube_mesh);    
}

// Sphere

internal Mesh
get_sphere_mesh(float32 radius, u32 u_subdivision, u32 v_subdivision) {
    Mesh mesh = {};

    float32 u_degrees = PI;
    float32 v_degrees = 2.0f * PI;

    float32 u_step = u_degrees / (float32)u_subdivision;
    float32 v_step = v_degrees / (float32)v_subdivision;

    mesh.vertices_count = (u_subdivision + 1) * (v_subdivision + 1);
    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);

    u32 vertices_index = 0;
    for (u32 u = 0; u <= u_subdivision; ++u) {
        for (u32 v = 0; v <= v_subdivision; ++v) {
            float32 u_f = (float32)u * u_step;
            float32 v_f = (float32)v * v_step;

            float32 inverse_radius = 1.0f / radius;
            Vector3 position = { 
                radius * sinf(u_f) * cosf(v_f), 
                radius * sinf(u_f) * sinf(v_f), 
                radius * cosf(u_f) 
            };
            Vector3 normal = position * inverse_radius;
            Vector2 texture_coords = { 
                1 - (v_f / v_degrees), 
                u_f / u_degrees
            };

            Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;
            vertices[vertices_index++] = { position, normal, texture_coords };
        }
    }

    mesh.indices_count = (u_subdivision * v_subdivision * 6) - (u_subdivision * 6);
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    u32 indices_index = 0;
    for (u32 u = 0; u < u_subdivision; u++) {
        u32 p1 = u * (v_subdivision + 1);
        u32 p2 = p1 + v_subdivision + 1;

        for (u32 v = 0; v < v_subdivision; v++, p1++, p2++) {
            if (u != 0) {
                mesh.indices[indices_index++] = p1;
                mesh.indices[indices_index++] = p2;
                mesh.indices[indices_index++] = p1 + 1;
            } 
            if (u != (u_subdivision - 1)) {
                mesh.indices[indices_index++] = p1 + 1;
                mesh.indices[indices_index++] = p2;
                mesh.indices[indices_index++] = p2 + 1;
            }
        }
    }

    mesh.vertex_info = get_vertex_xnu_info();
    gfx.init_mesh(&mesh);

    return mesh;
}

void GFX::draw_sphere(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });

    gfx.bind_shader(SHADER_COLOR3D);
    gfx.bind_descriptor_set(light_set);

    Descriptor object_desc = gfx.descriptor_set(5);
    gfx.update_ubo(object_desc, (void *)&color);
    gfx.bind_descriptor_set(object_desc);

    Matrix_4x4 model = create_transform_m4x4(coords, rotation_quat, dim);
    gfx.push_constants(SHADER_STAGE_VERTEX, (void *)&model, sizeof(Matrix_4x4));  

    gfx.draw_mesh(&sphere_mesh);    
}

void GFX::init_shapes() {
    rect_mesh = get_rect_mesh_2D();
    init_circle_mesh(&circle_mesh);
    sphere_mesh = get_sphere_mesh(0.025f, 10, 10);
    cube_mesh = get_cube_mesh();
    rect_3D_mesh = get_rect_mesh();
    triangle_3D_mesh = init_triangle_mesh();
}

// string

void GFX::draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    if (scale == 0.0f)
        return;

    float32 string_x_coord = 0.0f;

    float32 current_point = coords.x;
    float32 baseline      = coords.y;

    Texture_Atlas *atlas = &font->cache->atlas;
    
    bind_shader(SHADER_TEXT);
    bind_descriptor_sets(shapes_color_descriptor, &color);
    bind_descriptor_set(atlas->gpu[gfx.get_current_frame()].desc);

    Object object = {};
    Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });

    Font_Char *font_char = 0;
    Font_Char *font_char_next = 0;
    u32 string_index = 0;
    while(string[string_index] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[string_index], scale);
        
        font_char = bitmap->font_char;
        font_char_next = load_font_char(font, string[string_index + 1]);

        // Draw
        if (bitmap->bitmap.width != 0) {
            Vector2 coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };
            Vector2 dim = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height };

            texture_atlas_draw_rect(atlas, bitmap->index, coords, dim);
        }
        // End of Draw
        s32 kern = get_glyph_kern_advance(font->info, font_char->glyph_index, font_char_next->glyph_index);
        current_point += scale * (kern + font_char->ax);

        string_index++;
    }
    
}

// if length != -1 than the dim only includes chars up to the length position
String_Draw_Info GFX::get_string_draw_info(Font *font, const char *string, s32 length, float32 pixel_height) {
    String_Draw_Info info = {};    

    if (string == 0) {
        return info;
    } else if (font == 0) {
        logprint("get_string_draw_info()", "no font\n");
        return info;
    }

    stbtt_fontinfo *font_info = (stbtt_fontinfo*)font->info;
    float32 scale = stbtt_ScaleForPixelHeight(font_info, pixel_height);
    u32 index = 0;

    s32 top = 0;
    s32 bottom = 0;
    float32 left = 0;

    while (string[index] != 0) {
        if (length != -1) {
            // if a length is set then only include in the dim the chars up to that point
            if (index == length) break;
        }

        Font_Char_Bitmap *font_char_bitmap = load_font_char_bitmap(font, string[index], scale);
        Font_Char *font_char = font_char_bitmap->font_char;

        float32 char_coords_x = info.dim.x + (font_char->lsb * scale);
        if (char_coords_x < 0.0f)
            left = char_coords_x;

        if (top > font_char_bitmap->bb_0.y)
            top = font_char_bitmap->bb_0.y;

        if (bottom < font_char_bitmap->bb_1.y)
            bottom = font_char_bitmap->bb_1.y;
        
        if ((float32)-font_char_bitmap->bb_0.y > info.baseline.y)
            info.baseline.y = (float32)-font_char_bitmap->bb_0.y;

        int kern = stbtt_GetCodepointKernAdvance(font_info, string[index], string[index + 1]);
        if (string[index + 1] || string[index] == 32) // 32 == ' ' bitmap width is 0 so need advance
            info.dim.x += scale * float32(kern + font_char->ax);
        else
            info.dim.x += float32(font_char_bitmap->bb_1.x);
        index++;
    }

    info.baseline.x = -left;
    info.dim.y = float32(bottom - top);
    info.dim.x = float32(info.dim.x - left);
        
    s32 x0, y0, x1, y1;
    stbtt_GetFontBoundingBox(font_info, &x0, &y0, &x1, &y1);
    info.font_dim.x = float32(x1 - x0) * scale;
    info.font_dim.y = float32(y1 - y0) * scale;
    info.font_baseline.x = float32(x0) * scale;
    info.font_baseline.y = float32(y1) * scale;
    
    return info;
}

void draw_string_tl(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    String_Draw_Info string_info = gfx.get_string_draw_info(font, string, -1, pixel_height);
    coords.y += string_info.baseline.y;
    gfx.draw_string(font, string, coords, pixel_height, color);
}

//
// GFX
//

Layout* GFX::get_layouts() {
    return layouts;
}

u32 GFX::get_current_frame() {
    return vulkan_info.current_frame;
}

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
    if (resized)
        flags |= GFX_WINDOW_RESIZED;
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
