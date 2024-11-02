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

enum Resolution_Modes {
    RESOLUTION_480P,
    RESOLUTION_720P,
    RESOLUTION_1080P,
    RESOLUTION_2160P
};

//
// Defining render functions
//

// create identifier and then set function pointer
// t = api, n = name, a == args
//#define RENDER_FUNC_DECL(r, n, ...) r API3D_EXT(n)(__VA_ARGS__); r (*render_##n)(__VA_ARGS__) = &API3D_EXT(n)
/*
#define RENDER_FUNC_DECL(r, n, ...) r API3D_EXT(n)(__VA_ARGS__); r (*render_##n)(__VA_ARGS__)
RENDER_FUNC_DECL(bool8, sdl_init, SDL_Window *sdl_window);
RENDER_FUNC_DECL(void, clear_color, Vector4 color);
RENDER_FUNC_DECL(bool8, start_frame, App_Window *window);
RENDER_FUNC_DECL(void, end_frame, Assets *assets, App_Window *window);
RENDER_FUNC_DECL(void, cleanup);
RENDER_FUNC_DECL(void, create_graphics_pipeline, Shader *shader);
RENDER_FUNC_DECL(void, create_compute_pipeline, Shader *shader);
RENDER_FUNC_DECL(void, pipeline_cleanup, Render_Pipeline *pipe);
RENDER_FUNC_DECL(void, bind_pipeline, Render_Pipeline *pipeline);
RENDER_FUNC_DECL(void*, create_texture, Bitmap *bitmap, u32 texture_parameters);
RENDER_FUNC_DECL(void, delete_texture, Bitmap *bitmap);
RENDER_FUNC_DECL(void, destroy_texture, void *gpu_handle);
RENDER_FUNC_DECL(u32, set_bitmap, Descriptor *desc, Bitmap *bitmap);
RENDER_FUNC_DECL(u32, set_texture, Descriptor *desc, void *gpu_handle);
RENDER_FUNC_DECL(void, init_mesh, Mesh *mesh);
RENDER_FUNC_DECL(void, draw_mesh, Mesh *mesh);
RENDER_FUNC_DECL(void, set_viewport, u32 window_width, u32 window_height);
RENDER_FUNC_DECL(void, set_scissor, s32 x, s32 y, u32 window_width, u32 window_height);
RENDER_FUNC_DECL(void, assets_cleanup, Assets *assets);
RENDER_FUNC_DECL(void, wait_frame, );
RENDER_FUNC_DECL(void, depth_test, bool32 enable);
RENDER_FUNC_DECL(void, create_set_layout, Layout *layout);
RENDER_FUNC_DECL(void, allocate_descriptor_set, Layout *layout);
RENDER_FUNC_DECL(void, init_layout_offsets, Layout *layout, Bitmap *bitmap);
RENDER_FUNC_DECL(void, bind_descriptor_sets, Descriptor desc, void *data);
RENDER_FUNC_DECL(void, push_constants, u32 shader_stage, void *data, u32 data_size);
RENDER_FUNC_DECL(void, bind_descriptor_set, Descriptor desc);
RENDER_FUNC_DECL(void, update_ubo, Descriptor desc, void *data);
RENDER_FUNC_DECL(Descriptor, get_descriptor_set, u32 layout_id);
RENDER_FUNC_DECL(Descriptor, get_descriptor_set_index, u32 layout_id, u32 return_index);

RENDER_FUNC_DECL(void, immediate_vertex, Vertex_XU vertex);
RENDER_FUNC_DECL(void, draw_immediate, u32 vertices);

#define GFX_FUNC(r, n, ...) r API3D_EXT(n)(__VA_ARGS__); r (*gfx_##n)(__VA_ARGS__) = &API3D_EXT(n)

GFX_FUNC(void, bind_shader, const char *tag);
GFX_FUNC(void, create_buffer, GFX_Buffer *buffer);
GFX_FUNC(void, set_storage_buffer, GFX_Buffer *buffer, Descriptor desc);
GFX_FUNC(void, start_compute,);
GFX_FUNC(void, dispatch, u32 group_count_x, u32 group_count_y, u32 group_count_z);
GFX_FUNC(void, end_compute,);
GFX_FUNC(void, cleanup_layouts, Layout *layouts, u32 layouts_count);

void render_init_model(Model *model);
#define GFX_FUNC2(r, n, ...) r (*n)(__VA_ARGS__) = &API3D_EXT(n)
*/

#define GFX_FUNC(n) gfx.n = &API3D_EXT(n)

#define GFX_VSYNC              0b00000001
#define GFX_ANTI_ALIASING      0b00000010
#define GFX_RESOLUTION_SCALING 0b00000100
#define GFX_WINDOW_RESIZED     0b00010000

struct Render {
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

    // API3D
    bool8 (*sdl_init)(SDL_Window *sdl_window, u8 flags);
    void (*set_scissor)(s32 x, s32 y, u32 window_width, u32 window_height);
    void (*clear_color)(Vector4 color);
    bool8 (*start_frame)();
    void (*end_frame)(u8 flags);
    Descriptor (*get_descriptor_set)(Layout *layout);
    void (*recreate_swap_chain)(Vulkan_Info *info, Vector2_s32 window_dim, Vector2_s32 resolution, u8 flags, Assets *assets);
};

#define GFX Render
#define TEXTURE_ARRAY_SIZE   16

Render_Pipeline present_pipeline;


#endif // RENDER_H
