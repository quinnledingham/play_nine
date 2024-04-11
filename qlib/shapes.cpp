typedef enum {
    SHAPE_RECT,
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
    Mesh rect_3D_mesh;
    Mesh sphere_mesh;
    Mesh cube_mesh;

    Shader *color_shader;
    Shader *texture_shader;
    Shader *text_shader;
    
    Render_Pipeline color_pipeline;
    Render_Pipeline texture_pipeline;
    Render_Pipeline text_pipeline;
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
        
        // NOT CORRECT
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
    mesh->indices[indices_index++] = 1;
    mesh->indices[indices_index++] = circle_vertices;
    
    render_init_mesh(mesh);
}

/*
void draw_circle(v2 coords, r32 rotation, r32 radius, v4 color) {
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    v3 dim_v3 = { radius, radius, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_CIRCLE;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape);
}
*/
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
    render_bind_pipeline(&color_pipeline);
    Descriptor object_desc = render_get_descriptor_set(&layouts[5]);
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
    vertices[0] = { {-0.5, -0.5, -0.5}, {1, 0, 1}, {0, 0} }; // bottom left
    vertices[1] = { {-0.5,  0.5, -0.5}, {1, 0, 1}, {0, 1} }; // top left
    vertices[2] = { { 0.5, -0.5, -0.5}, {1, 0, 1}, {1, 0} }; // bottom right
    vertices[3] = { { 0.5,  0.5, -0.5}, {1, 0, 1}, {1, 1} }; // top right
    
    // forward
    vertices[4] = { {-0.5, -0.5, 0.5}, {0, 0, -1}, {0, 0} }; // bottom left
    vertices[5] = { {-0.5,  0.5, 0.5}, {0, 0, -1}, {0, 1} }; // top left
    vertices[6] = { { 0.5, -0.5, 0.5}, {0, 0, -1}, {1, 0} }; // bottom right
    vertices[7] = { { 0.5,  0.5, 0.5}, {0, 0, -1}, {1, 1} }; // top right
    /*
    // back
    mesh.vertices[0] = { {-1.0, -1.0, -1.0}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[1] = { {-1.0,  1.0, -1.0}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[2] = { { 1.0, -1.0, -1.0}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[3] = { { 1.0,  1.0, -1.0}, {0, 0, 1}, {1, 1} }; // top right
    
    // forward
    mesh.vertices[4] = { {-1.0, -1.0, 1.0}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[5] = { {-1.0,  1.0, 1.0}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[6] = { { 1.0, -1.0, 1.0}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[7] = { { 1.0,  1.0, 1.0}, {0, 0, 1}, {1, 1} }; // top right
    */
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

    render_bind_pipeline(&color_pipeline);
    render_bind_descriptor_set(light_set);

    Descriptor object_set = render_get_descriptor_set(&layouts[5]);
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
    shapes.rect_3D_mesh = get_rect_mesh();
    shapes.sphere_mesh = get_sphere_mesh(0.025f, 10, 10);
    shapes.cube_mesh = get_cube_mesh(true);

    // Text Pipeline
    shapes.text_shader = find_shader(assets, "TEXT");

    Layout_Set set = {};
    init_basic_vert_layout(&shapes.text_shader->set, layouts);
    init_text_frag_layout(shapes.text_shader, layouts);

    shapes.text_pipeline.shader = shapes.text_shader;
    shapes.text_pipeline.depth_test = false;
    render_create_graphics_pipeline(&shapes.text_pipeline, get_vertex_xu_info());

    // Color Pipeline
    shapes.color_shader = find_shader(assets, "COLOR");

    init_basic_vert_layout(&shapes.color_shader->set, layouts);
    init_color_frag_layout(shapes.color_shader, layouts);

    shapes.color_pipeline.shader = shapes.color_shader;
    shapes.color_pipeline.depth_test = false;
    render_create_graphics_pipeline(&shapes.color_pipeline, get_vertex_xu_info());

    // Texture Pipeline
    shapes.texture_shader = find_shader(assets, "TEXTURE");

    init_basic_vert_layout(&shapes.texture_shader->set, layouts);
    init_texture_frag_layout(shapes.texture_shader, layouts);

    shapes.texture_pipeline.shader = shapes.texture_shader;
    shapes.texture_pipeline.depth_test = false;
    render_create_graphics_pipeline(&shapes.texture_pipeline, get_vertex_xu_info());
}

void cleanup_shapes() {
    render_pipeline_cleanup(&shapes.text_pipeline);
    render_pipeline_cleanup(&shapes.color_pipeline);
    render_pipeline_cleanup(&shapes.texture_pipeline);
}

internal void
draw_shape(Shape shape) {
    Object object = {};

    switch(shape.draw_type) {
        case Shape_Draw_Type::COLOR: {
            render_bind_pipeline(&shapes.color_pipeline);

            Descriptor v_set = render_get_descriptor_set(&layouts[4]);
            render_update_ubo(v_set, &shape.color);
            render_bind_descriptor_set(v_set);
        } break;

        case Shape_Draw_Type::TEXTURE: {
            render_bind_pipeline(&shapes.texture_pipeline);

            Descriptor desc = render_get_descriptor_set(&layouts[3]);
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

    render_bind_pipeline(&shapes.text_pipeline);

    u32 i = 0;

    Descriptor v_color_set = render_get_descriptor_set(&layouts[4]);
    render_update_ubo(v_color_set, (void *)&color);
    render_bind_descriptor_set(v_color_set);

    Object object = {};
    Descriptor desc = render_get_descriptor_set(&layouts[2]);

    s32 indices[60];
    platform_memory_set(indices, 0, sizeof(u32) * 60);

    while(string[i] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[i], scale);
        if (bitmap->bitmap.width != 0) {   
            indices[i] = render_set_bitmap(&desc, &bitmap->bitmap);
        }
        i++;
    }

    render_bind_descriptor_set(desc);

    i = 0;
    while (string[i] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[i], scale);
        Font_Char *font_char = bitmap->font_char;
        
        // Draw
        if (bitmap->bitmap.width != 0) {    
            Vector2 char_coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };
        
            Vector3 coords_v3 = { char_coords.x, char_coords.y, 0 };
            Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
            Vector3 dim_v3 = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height, 1 };

            coords_v3.x += dim_v3.x / 2.0f;
            coords_v3.y += dim_v3.y / 2.0f; // coords = top left corner
    		object.model = create_transform_m4x4(coords_v3, rotation_quat, dim_v3);
            object.index = indices[i];
            render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));   
            
            render_draw_mesh(&shapes.rect_mesh);
            // End of Draw
        }
        s32 kern = get_codepoint_kern_advance(font->info, string[i], string[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }   
}

void draw_string_tl(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    Vector2 text_dim = get_string_dim(font, string, pixel_height, color);
    //draw_rect(coords, 0.0f, text_dim, { 255, 255, 0, 1 });
    coords.y += text_dim.y;
    draw_string(font, string, coords, pixel_height, color);
}