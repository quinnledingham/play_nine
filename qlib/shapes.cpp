typedef enum {
    SHAPE_RECT,
    SHAPE_TRIANGLE,
    SHAPE_CIRCLE,
    SHAPE_CUBE,
    SHAPE_SPHERE,
} Shape_Type;

enum struct Shape_Draw_Type {
    COLOR,
    TEXTURE,
    TEXT,
};

struct Shape {
    Shape_Type type;
    Vector3 coords;
    Quaternion rotation;
    Vector3 dim;

    Shape_Draw_Type draw_type;
    Vector4 color;
    Bitmap *bitmap;
};

struct Shapes {
    Mesh rect_mesh;
    Mesh circle_mesh;
    Mesh rect_3D_mesh;
    Mesh triangle_3D_mesh;
    Mesh sphere_mesh;
    Mesh cube_mesh;    
};

Shapes shapes = {};

void init_shapes(Shader *color, Shader *texture, Shader *text);
void draw_shape(Shape shape);

//
// Rect
//

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
    render_init_mesh(&mesh);

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
    render_init_mesh(&mesh);

    return mesh;
}

void draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Vector4 color) {
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

void draw_rect(Rect rect, Vector4 color) {
    draw_rect(rect.coords, rect.rotation, rect.dim, color);
};

void draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Bitmap *bitmap) {
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


//
// Triangle
//

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
    render_init_mesh(&mesh);

    return mesh;
}
 
void draw_triangle(Vector3 coords, Vector3 rotation, Vector3 dim, Vector4 color) {
    Quaternion rotation_quat_x = get_rotation(rotation.x, { 1, 0, 0 });
    Quaternion rotation_quat_y = get_rotation(rotation.y, { 0, 1, 0 });
    Quaternion rotation_quat_z = get_rotation(rotation.z, { 0, 0, 1 });
    Quaternion rotation_quat = rotation_quat_x * rotation_quat_y * rotation_quat_z;

    //Shader *shader = find_shader(global_assets, "COLOR3D");
    //render_bind_pipeline(&shader->pipeline);
    gfx_bind_shader("COLOR3D");
    render_bind_descriptor_set(light_set);
    
    Descriptor color_desc = render_get_descriptor_set(5);
    render_update_ubo(color_desc, (void *)&color);
    render_bind_descriptor_set(color_desc);

    Object object = {};
    object.model = create_transform_m4x4(coords, rotation_quat, dim);
    render_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));  

    render_draw_mesh(&shapes.triangle_3D_mesh);    
}

//
// Circle
//

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
    render_init_mesh(mesh);
}

void draw_circle(Vector2 coords, float32 rotation, float32 radius, Vector4 color) {
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

//
// Sphere
//

Mesh get_sphere_mesh(float32 radius, u32 u_subdivision, u32 v_subdivision) {
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
    render_init_mesh(&mesh);

    return mesh;
}

void draw_sphere(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
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
    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });

    gfx_bind_shader("COLOR3D");
    render_bind_descriptor_set(light_set);

    Descriptor object_desc = render_get_descriptor_set(5);
    render_update_ubo(object_desc, (void *)&color);
    render_bind_descriptor_set(object_desc);

    Matrix_4x4 model = create_transform_m4x4(coords, rotation_quat, dim);
    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model, sizeof(Matrix_4x4));  

    render_draw_mesh(&shapes.sphere_mesh);    
}

//
// Cube
//

Mesh get_cube_mesh(bool32 out) {
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
    
    if (out)
    {
        init_rect_indices(mesh.indices + 0,  3, 1, 2, 0); // back
        init_rect_indices(mesh.indices + 6,  5, 7, 4, 6); // front
        init_rect_indices(mesh.indices + 12, 1, 3, 5, 7); // top
        init_rect_indices(mesh.indices + 18, 4, 6, 0, 2); // bottom
        init_rect_indices(mesh.indices + 24, 1, 5, 0, 4); // left
        init_rect_indices(mesh.indices + 30, 7, 3, 6, 2); // right
    }
    else
    {
        init_rect_indices(mesh.indices + 0,  1, 3, 0, 2); // back
        init_rect_indices(mesh.indices + 6,  7, 5, 6, 4); // front
        init_rect_indices(mesh.indices + 12, 5, 7, 1, 3); // top
        init_rect_indices(mesh.indices + 18, 0, 2, 4, 6); // bottom
        init_rect_indices(mesh.indices + 24, 5, 1, 4, 0); // left
        init_rect_indices(mesh.indices + 30, 3, 7, 2, 6); // right
    }

    mesh.vertex_info = get_vertex_xnu_info();
    render_init_mesh(&mesh);
    
    return mesh;
}
Mesh get_cube_mesh() { return get_cube_mesh(true); }

void draw_cube(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
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

    gfx_bind_shader("COLOR3D");
    render_bind_descriptor_set(light_set);

    Descriptor object_set = render_get_descriptor_set(5);
    render_update_ubo(object_set, (void *)&color);
    render_bind_descriptor_set(object_set);

    Quaternion rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    Matrix_4x4 model = create_transform_m4x4(coords, rotation_quat, dim);
    render_push_constants(SHADER_STAGE_VERTEX, (void *)&model, sizeof(Matrix_4x4)); 

    render_draw_mesh(&shapes.cube_mesh);    
}


//
// Shapes
//

void init_shapes(Assets *assets) {
    // Meshes
    shapes.rect_mesh = get_rect_mesh_2D();
    init_circle_mesh(&shapes.circle_mesh);
    shapes.rect_3D_mesh = get_rect_mesh();
    shapes.triangle_3D_mesh = init_triangle_mesh();
    shapes.sphere_mesh = get_sphere_mesh(0.025f, 10, 10);
    shapes.cube_mesh = get_cube_mesh(true);
}

global Descriptor shapes_color_descriptor;

void draw_shape(Shape shape) {
    Object object = {};

    switch(shape.draw_type) {
        case Shape_Draw_Type::COLOR: {
            Shader *shader = find_shader(global_assets, "COLOR");
            render_bind_pipeline(&shader->pipeline);

            //Descriptor v_set = render_get_descriptor_set(&layouts[4]);
            //render_update_ubo(v_set, &shape.color);
            //render_bind_descriptor_set(v_set);
            //render_bind_descriptor_sets(v_set, &shape.color);
            render_bind_descriptor_sets(shapes_color_descriptor, &shape.color);
        } break;

        case Shape_Draw_Type::TEXTURE: {
            gfx_bind_shader("TEXTURE");

            Descriptor desc = render_get_descriptor_set(3);
            object.index = render_set_bitmap(&desc, shape.bitmap);
            render_bind_descriptor_set(desc);
        } break;

        case Shape_Draw_Type::TEXT: {
            
        } break;
    }

    if (shape.type == SHAPE_RECT || shape.type == SHAPE_CIRCLE) {
        shape.coords.x += shape.dim.x / 2.0f;
        shape.coords.y += shape.dim.y / 2.0f; // coords = top left corner
    }

    object.model = create_transform_m4x4(shape.coords, shape.rotation, shape.dim);
    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    switch(shape.type) {
        case SHAPE_RECT:   render_draw_mesh(&shapes.rect_mesh);   break;
        case SHAPE_CIRCLE: render_draw_mesh(&shapes.circle_mesh); break;
        case SHAPE_SPHERE: render_draw_mesh(&shapes.sphere_mesh); break;
        default: logprint("draw_shape()", "not a valid shape type\n");
    }
}

//
// String
//

void draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    if (scale == 0.0f)
        return;

    float32 string_x_coord = 0.0f;

    float32 current_point = coords.x;
    float32 baseline      = coords.y;

    gfx_bind_shader("TEXT");
    render_bind_descriptor_sets(shapes_color_descriptor, &color);

    Object object = {};
    s32 indices[TEXTURE_ARRAY_SIZE];
    Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
    
    u32 string_index = 0;
    while(string[string_index] != 0) {
        Descriptor desc = render_get_descriptor_set(2);

        u32 indices_index = 0;
        platform_memory_set(indices, 0, sizeof(u32) * TEXTURE_ARRAY_SIZE);

        while(string[string_index + indices_index] != 0) {
            Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[string_index + indices_index], scale);
            if (bitmap->bitmap.width != 0) {   
                indices[indices_index] = render_set_bitmap(&desc, &bitmap->bitmap);
            }
            indices_index++;

            if (indices_index >= TEXTURE_ARRAY_SIZE) {
                //logprint("draw_string()", "string (%s) too long (max: %d)\n", string, TEXTURE_ARRAY_SIZE);
                break;
            }
        }
    
        render_bind_descriptor_set(desc);

        Font_Char *font_char = 0;
        Font_Char *font_char_next = 0;

        u32 i = 0;
        while (i < indices_index) {
            Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[string_index + i], scale);
        
            font_char = bitmap->font_char;
            font_char_next = load_font_char(font, string[string_index + i + 1]);
        
            // Draw
            if (bitmap->bitmap.width != 0) {    
                Vector2 char_coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };
        
                Vector3 coords_v3 = { char_coords.x, char_coords.y, 0 };
                Vector3 dim_v3 = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height, 1 };

                coords_v3.x += dim_v3.x / 2.0f;
                coords_v3.y += dim_v3.y / 2.0f; // coords = top left corner
            		object.model = create_transform_m4x4(coords_v3, rotation_quat, dim_v3);
                object.index = indices[i];
                
                render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));   
            
                render_draw_mesh(&shapes.rect_mesh);
            }
            // End of Draw
            s32 kern = get_glyph_kern_advance(font->info, font_char->glyph_index, font_char_next->glyph_index);
            current_point += scale * (kern + font_char->ax);
        
            i++;
        }   

        string_index += i;
    }
}

/*
(0, 0)
 -> ###############################
    #   #                         #
    # r #            r            #
    # b #                         #  
    #   #                         #
    ###############################
    # r #            r            #
    ############################### 
*/
struct String_Draw_Info {
    Vector2 dim;
    Vector2 baseline;

    Vector2 font_dim; // biggest char in font
    Vector2 font_baseline; // baseline for biggest char
};

// if length != -1 than the dim only includes chars up to the length position
internal String_Draw_Info
get_string_draw_info(Font *font, const char *string, s32 length, float32 pixel_height) {
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
    String_Draw_Info string_info = get_string_draw_info(font, string, -1, pixel_height);
    coords.y += string_info.baseline.y;
    draw_string(font, string, coords, pixel_height, color);
}

internal void
draw_string_draw_info(String_Draw_Info *info, Vector2 coords) {
    Vector2 string = coords;
    string.y -= info->baseline.y;
    draw_rect(string, 0.0f, info->dim, { 250, 100, 0, 1 });

    Vector2 font = coords;
    font.y -= info->font_baseline.y;
    draw_rect(font, 0.0f, info->font_dim, { 200, 0, 0, 1 });

    Vector2 baseline = info->baseline;
    baseline.x = info->dim.x;
    draw_rect(string, 0.0f, baseline, { 100, 10, 40, 1 });
}
