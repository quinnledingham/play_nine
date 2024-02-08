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

internal Mesh
get_rect_mesh() {
	Mesh mesh = {};
	mesh.vertices_count = 4;
    
	mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
    Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;
/*
	mesh.vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    mesh.vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    mesh.vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    mesh.vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
*/
    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, -1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, -1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, -1}, {1, 0} };
    vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, -1}, {1, 1} };

    mesh.indices_count = 6;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    // vertex locations
    u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
   
	mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_left;
    mesh.indices[2] = bottom_right;
    mesh.indices[3] = top_left;
    mesh.indices[4] = bottom_right;
    mesh.indices[5] = top_right;
    
/*
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_right;
    mesh.indices[2] = bottom_left;
    mesh.indices[3] = top_left;
    mesh.indices[4] = top_right;
    mesh.indices[5] = bottom_right;
*/

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

//
// Shapes
//

void init_shapes(Assets *assets) {
	shapes.rect_mesh = get_rect_mesh_2D();

    //shapes.text_shader.files[SHADER_STAGE_VERTEX].filepath = "../assets/shaders/2D.vert";
    //shapes.text_shader.files[SHADER_STAGE_FRAGMENT].filepath = "../assets/shaders/text.frag";
    shapes.text_shader = find_shader(assets, "TEXT");
    //load_shader(shapes.text_shader);
	render_compile_shader(shapes.text_shader);

    init_basic_vert_layout(shapes.text_shader);
    init_text_frag_layout(shapes.text_shader);

    shapes.text_pipeline.shader = shapes.text_shader;
    shapes.text_pipeline.depth_test = false;
    render_create_graphics_pipeline(&shapes.text_pipeline, get_vertex_xu_info());

    //shapes.color_shader.files[SHADER_STAGE_VERTEX].filepath = "../assets/shaders/2D.vert";
    //shapes.color_shader.files[SHADER_STAGE_FRAGMENT].filepath = "../assets/shaders/color.frag";
    shapes.color_shader = find_shader(assets, "COLOR");
    //load_shader(shapes.color_shader);
    render_compile_shader(shapes.color_shader);

    init_basic_vert_layout(shapes.color_shader);
    init_color_frag_layout(shapes.color_shader);

    shapes.color_pipeline.shader = shapes.color_shader;
    shapes.color_pipeline.depth_test = false;
    render_create_graphics_pipeline(&shapes.color_pipeline, get_vertex_xu_info());
}

internal void
draw_shape(Shape shape) {
    Shader *shader = 0;
    Descriptor_Set *set = 0;
    switch(shape.draw_type) {
        case Shape_Draw_Type::COLOR: {
            render_bind_pipeline(&shapes.color_pipeline);
            shader = shapes.color_pipeline.shader;
            set = render_get_descriptor_set(shapes.color_pipeline.shader, 1);
            render_bind_descriptor_set(set, 1);   
            render_update_ubo(set, 0, (void*)&shape.color, false);
        } break;

        case Shape_Draw_Type::TEXT: {
            
        } break;
    }

    if (shape.type == SHAPE_RECT || shape.type == SHAPE_CIRCLE) {
        shape.coords.x += shape.dim.x / 2.0f;
        shape.coords.y += shape.dim.y / 2.0f; // coords = top left corner
    }

    Matrix_4x4 model = create_transform_m4x4(shape.coords, shape.rotation, shape.dim);
    render_push_constants(&shader->layout_sets[2], (void *)&model);  

    switch(shape.type) {
        case SHAPE_RECT: render_draw_mesh(&shapes.rect_mesh); break;
        default: logprint("draw_shape()", "not a valid shape type\n");
    }
}

//
// String
//

void draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    //stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    float32 string_x_coord = 0.0f;

    float32 current_point = coords.x;
    float32 baseline      = coords.y;

    render_bind_pipeline(&shapes.text_pipeline);

    u32 i = 0;
    while (string[i] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[i], scale);
        Font_Char *font_char = bitmap->font_char;
        
        // Draw
        if (bitmap->bitmap.width != 0) {    
            Vector2 char_coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };
        
            Vector3 coords_v3 = { char_coords.x, char_coords.y, 0 };
            Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
            Vector3 dim_v3 = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height, 1 };
            
            Descriptor_Set *object_set = render_get_descriptor_set(shapes.text_shader, 1);
            render_bind_descriptor_set(object_set, 1);   
            render_set_bitmap(object_set, &bitmap->bitmap, 1);
            render_update_ubo(object_set, 1, (void*)&color, false);

            coords_v3.x += dim_v3.x / 2.0f;
            coords_v3.y += dim_v3.y / 2.0f; // coords = top left corner
    		Matrix_4x4 model = create_transform_m4x4(coords_v3, rotation_quat, dim_v3);
            render_push_constants(&shapes.text_shader->layout_sets[2], (void *)&model);  
                     
            render_draw_mesh(&shapes.rect_mesh);
            // End of Draw
        }
        s32 kern = get_codepoint_kern_advance(font->info, string[i], string[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }   
}
