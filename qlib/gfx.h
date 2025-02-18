#ifndef RENDER_H
#define RENDER_H

struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    Matrix_4x4 model;
    s32 index;
};

typedef Object GFX_Object;

struct Light {
    Vector4 position;
    Vector4 ambient;
    Vector4 diffuse;
    Vector4 specular;
    Vector4 color;
    bool enabled;
};

union Camera {
    struct {
        Vector3 position;
        Vector3 target;
        Vector3 up;
        float32 fov;
        float64 yaw;
        float64 pitch;
    };
    float32 E[12];
};

// used with render_create_texture function
enum Texture_Parameters {
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

struct GFX_Buffer {

    void *data;
    u32 offset;
    u32 size;

#ifdef VULKAN
    Vulkan_Buffer vulkan;
#endif // VULKAN
      
};

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

enum Resolution_Modes {
    RESOLUTION_480P,
    RESOLUTION_720P,
    RESOLUTION_1080P,
    RESOLUTION_2160P
};

#define GFX_VSYNC              0b00000001
#define GFX_ANTI_ALIASING      0b00000010
#define GFX_RESOLUTION_SCALING 0b00000100
#define GFX_WINDOW_RESIZED     0b00010000

struct GFX {
    bool8 resized;
    Vector2_s32 window_dim;
    bool8 vsync = TRUE;
    bool8 anti_aliasing = TRUE;
    bool8 resolution_scaling = false;

    // Resolution scaling
    Vector2_s32 resolution;
    u32 resolution_mode = RESOLUTION_2160P;
    float32 resolution_scale = 1.0f;

    //bool8 depth_test
    Layout *layouts;

    // Scissor
    u32 scissor_stack_index = 0;
    Rect scissor_stack[10];

    Assets *assets;

    //
    // Functions
    //

    bool8 sdl_init(SDL_Window *sdl_window);

    Vector2_s32 get_resolution(float32 scale);
    void set_resolution(Vector2_s32 new_resolution);
    void update_resolution();
    
    void set_window_dim(Vector2_s32 dim);

    void scissor_push(Vector2 coords, Vector2 dim);
    void scissor_pop();

    u8 get_flags();

    Descriptor descriptor_set(u32 layout_id);
    Descriptor descriptor_set_index(u32 layout_id, u32 return_index);
    void create_swap_chain();
    u32 get_current_frame();
    Layout* get_layouts();

    bool8 start_frame();

    // Draw
    Mesh rect_mesh;
    Mesh circle_mesh;

    Mesh sphere_mesh;
    Mesh cube_mesh;    
    Mesh rect_3D_mesh;
    Mesh triangle_3D_mesh;

    void init_shapes();
    void draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Vector4 color);
    void draw_rect(Rect rect, Vector4 color);
    void draw_rect(Vector2 coords, float32 rotation, Vector2 dim, Bitmap *bitmap);
    void draw_circle(Vector2 coords, float32 rotation, float32 radius, Vector4 color);
    void draw_triangle(Vector3 coords, Vector3 rotation, Vector3 dim, Vector4 color);
    void draw_cube(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color);
    void draw_sphere(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color);

    String_Draw_Info get_string_draw_info(Font *font, const char *string, s32 length, float32 pixel_height);
    void draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color);

    // API3D
    void (*recreate_swap_chain)(Vulkan_Info *info, Vector2_s32 window_dim, Vector2_s32 resolution, u8 flags, Assets *assets);

    void (*set_scissor)(s32 x, s32 y, u32 window_width, u32 window_height);
    void (*set_viewport)(u32 window_width, u32 window_height);
    void (*depth_test)(bool32 enable);
    void (*clear_color)(Vector4 color);
    //bool8 (*start_frame)();
    void (*end_frame)(u8 flags);
    void (*start_compute)();
    void (*end_compute)();
    void (*dispatch)(u32 group_count_x, u32 group_count_y, u32 group_count_z);

    Descriptor (*get_descriptor_set)(Layout *layout);
    void (*bind_descriptor_set)(Descriptor desc);
    void (*bind_descriptor_sets)(Descriptor desc, void *data);
    void (*create_set_layout)(Layout *layout);
    void (*allocate_descriptor_set)(Layout *layout);
    void (*init_layout_offsets)(Layout *layout, Bitmap *bitmap);
    
    void* (*create_texture)(Bitmap *bitmap, u32 texture_parameters);
    void (*destroy_texture)(void *gpu_handle);
    u32 (*set_texture)(Descriptor *desc, void *gpu_handle);
    void (*delete_texture)(Bitmap *bitmap);
    u32 (*set_bitmap)(Descriptor *desc, Bitmap *bitmap);

    void (*immediate_vertex_xnu)(Vertex_XNU vertex);
    void (*immediate_vertex_xu)(Vertex_XU vertex);
    void (*draw_immediate)(u32 vertices);

    void (*push_constants)(u32 shader_stage, void *data, u32 data_size);
    void (*bind_shader)(u32 id);
    void (*create_graphics_pipeline)(Shader *shader);
    void (*create_compute_pipeline)(Shader *shader);

    void (*init_mesh)(Mesh *mesh);
    void (*draw_mesh)(Mesh *mesh);
    void (*update_ubo)(Descriptor desc, void *data);

    void (*create_buffer)(GFX_Buffer *buffer);
    void (*set_storage_buffer)(GFX_Buffer *buffer, Descriptor desc);

    void (*assets_cleanup)(Assets *assets);
    void (*cleanup_layouts)(Layout *layouts, u32 layouts_count);
    void (*wait_frame)();
    void (*cleanup)();
};

#define TEXTURE_ARRAY_SIZE   16

Render_Pipeline present_pipeline;
Descriptor light_set;
Descriptor light_set_2;

#endif // RENDER_H
