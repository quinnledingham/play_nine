struct Shapes {
	Mesh rect_mesh;

    Shader text_shader;
	Render_Pipeline text_pipeline;
};

Shapes shapes = {};

const char *basic_vs = "#version 330 core\nlayout (location = 0) in vec3 position;layout (location = 1) in vec3 normal;layout (location = 2) in vec2 texture_coords;out vec2 uv;layout (std140) uniform Matrices{mat4 projection;mat4 view;};uniform mat4 model; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";
const char *color_fs = "#version 330 core\nuniform vec4 user_color;in vec2 uv; out vec4 FragColor;void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";
const char *tex_fs   = "#version 330 core\nuniform sampler2D tex0;in vec2 uv;out vec4 FragColor;void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";
const char *text_fs  = "#version 330 core\nin vec2 uv;out vec4 FragColor;uniform sampler2D tex0;uniform vec4 text_color;void main() { vec3 norm_text_color = vec3(text_color.x/255, text_color.y/255, text_color.z/255);float alpha = texture(tex0, uv).r * text_color.a;vec4 tex = vec4(1.0, 1.0, 1.0, alpha); FragColor = vec4(norm_text_color, 1.0) * tex;}";

//
// Rect
//

internal Mesh
get_rect_mesh() {
	Mesh mesh = {};
	mesh.vertices_count = 4;
	mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
/*
	mesh.vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    mesh.vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    mesh.vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    mesh.vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
*/
    mesh.vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    mesh.vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    mesh.vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    mesh.vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };

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
    render_init_mesh(&mesh);

    return mesh;
}

//
// Shapes
//

void init_shapes() {
	shapes.rect_mesh = get_rect_mesh();

    shapes.text_shader.files[SHADER_STAGE_VERTEX].filepath = "../assets/shaders/basic.vert";
    shapes.text_shader.files[SHADER_STAGE_FRAGMENT].filepath = "../assets/shaders/text.frag";
    load_shader(&shapes.text_shader);
	//shapes.text_shader.files[SHADER_STAGE_VERTEX].memory = (void*)basic_vs;
	//shapes.text_shader.files[SHADER_STAGE_FRAGMENT].memory = (void*)text_fs;

	render_compile_shader(&shapes.text_shader);

    shapes.text_shader.layout_sets[0].descriptors[0] = Descriptor(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, sizeof(Scene), descriptor_scope::GLOBAL);
    shapes.text_shader.layout_sets[0].descriptors_count = 1;
    render_create_descriptor_pool(&shapes.text_shader, 30, 0);

    shapes.text_shader.layout_sets[1].descriptors[0] = Descriptor(1, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, sizeof(Object), descriptor_scope::GLOBAL);
    shapes.text_shader.layout_sets[1].descriptors[1] = Descriptor(2, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 0, descriptor_scope::GLOBAL);
    shapes.text_shader.layout_sets[1].descriptors[2] = Descriptor(3, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, sizeof(Vector4), descriptor_scope::GLOBAL);
    shapes.text_shader.layout_sets[1].descriptors_count = 3;
    render_create_descriptor_pool(&shapes.text_shader, 30, 1);

    shapes.text_pipeline.shader = &shapes.text_shader;
    render_create_graphics_pipeline(&shapes.text_pipeline);
}

//
// String
//

internal void
vulkan_set_bitmap(Descriptor_Set *set, Bitmap *bitmap, u32 binding);

void draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color)
{
    //stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    float32 string_x_coord = 0.0f;

    float32 current_point = coords.x;
    float32 baseline      = coords.y;

    render_bind_pipeline(&shapes.text_pipeline);

    //uniform_s32(shader_handle, "tex0", 0);
    //uniform_Vector4(shader_handle, "text_color", color);

    u32 i = 0;
    while (string[i] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[i], scale);
        Font_Char *font_char = bitmap->font_char;
        
        Vector2 char_coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };

        // Draw
        Vector3 coords_Vector3 = { char_coords.x, char_coords.y, 0 };
        Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
        Vector3 dim_Vector3 = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height, 1 };


        if (bitmap->bitmap.width != 0) {        
    		//platform_set_texture(bitmap);
            Descriptor_Set *object_set = render_get_descriptor_set(&shapes.text_shader, 1);
            vulkan_set_bitmap(object_set, &bitmap->bitmap, 2);

    		Matrix_4x4 model = create_transform_m4x4(coords_Vector3, rotation_quat, dim_Vector3);

        	//uniform_m4x4(shader_handle, "model", &model);
            render_update_ubo(object_set, 2, (void*)&color, false);
            render_update_ubo(object_set, 0, (void*)&model, false);
            render_bind_descriptor_set(object_set, 1);
            render_draw_mesh(&shapes.rect_mesh);
            // End of Draw
        }
        s32 kern = get_codepoint_kern_advance(font->info, string[i], string[i + 1]);
        current_point += scale * (kern + font_char->ax);
        
        i++;
    }   
}
