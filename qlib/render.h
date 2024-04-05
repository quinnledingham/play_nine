struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    Matrix_4x4 model;
};


struct Render_Pipeline {
    Shader *shader;
    bool8 blend;
    bool8 depth_test = TRUE;
    bool8 wireframe;

#ifdef VULKAN
    //VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
#endif

    void *gpu_info; // OpenGL = NULL, Vulkan = VkPipeline
};

struct Render {
    bool8 vsync;
};

global Render render_info = {};
Descriptor_Set *light_set;
Descriptor_Set *light_set_2;

struct Light {
    Vector4 position;
    Vector4 ambient;
    Vector4 diffuse;
    Vector4 specular;
    Vector4 color;
    bool enabled;
};

Light global_light = {
    { 0.0f, 5.0f, 0.0f, 1.0f },
    { 0.4f, 0.4f, 0.4f, 1.0f },
    { 0.9f, 0.9f, 0.9f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    true,
};

Light global_light_2 = {
    { 0.0f, 5.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 0.9f, 0.9f, 0.9f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    false
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

inline Matrix_4x4 get_view(Camera camera)  { 
    return look_at(camera.position, camera.position + camera.target, camera.up); 
}

// used with render_create_texture function
enum Texture_Parameters {
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

//
// Defining render functions
//

#if OPENGL

#define GPU_EXT(n) opengl_##n

#elif VULKAN

#define GPU_EXT(n) vulkan_##n

#elif DX12

#define GPU_EXT(n) dx12_##n

#endif // OPENGL / VULKAN / DXX12

// create identifier and then set function pointer
// t = api, n = name, a == args
#define RENDER_FUNC(r, n, ...) r GPU_EXT(n)(__VA_ARGS__); r (*render_##n)(__VA_ARGS__) = &GPU_EXT(n)

RENDER_FUNC(void, sdl_init, SDL_Window *sdl_window);
RENDER_FUNC(void, clear_color, Vector4 color);
RENDER_FUNC(void, start_frame, );
RENDER_FUNC(void, end_frame, );
RENDER_FUNC(void, cleanup);
RENDER_FUNC(void, create_graphics_pipeline, Render_Pipeline *pipeline, Vertex_Info vertex_info);
RENDER_FUNC(void, pipeline_cleanup, Render_Pipeline *pipe);
RENDER_FUNC(void, bind_pipeline, Render_Pipeline *pipeline);

RENDER_FUNC(void, create_descriptor_pool, Shader *shader, u32 descriptor_set_count, u32 set_index);
RENDER_FUNC(Descriptor_Set*, get_descriptor_set, Shader *shader, bool8 layout_index);

RENDER_FUNC(void, create_texture, Bitmap *bitmap, u32 texture_parameters);
RENDER_FUNC(void, delete_texture, Bitmap *bitmap);
RENDER_FUNC(void, set_bitmap, Descriptor_Set *set, Bitmap *bitmap, u32 binding);

void render_init_model(Model *model);
//RENDER_FUNC(void, init_model, Model *model);
//RENDER_FUNC(void, draw_model, Model *model, Shader *shader, Vector3 position, Quaternion rotation);

RENDER_FUNC(void, update_ubo, Descriptor_Set *set, u32 descriptor_index, void *data, bool8 static_update);
RENDER_FUNC(void, push_constants, Descriptor_Set *push_constants, void *data);

// this is when you tell the shader where the memory is
RENDER_FUNC(void, bind_descriptor_set, Descriptor_Set *set, u32 first_set);

RENDER_FUNC(void, init_mesh, Mesh *mesh);
RENDER_FUNC(void, draw_mesh, Mesh *mesh);
RENDER_FUNC(void, set_viewport, u32 window_width, u32 window_height);
RENDER_FUNC(void, set_scissor, s32 x, s32 y, u32 window_width, u32 window_height);

RENDER_FUNC(void, compile_shader, Shader *shader);

RENDER_FUNC(void, reset_descriptor_sets, Assets *assets);
RENDER_FUNC(void, assets_cleanup, Assets *assets);
RENDER_FUNC(void, wait_frame, );

RENDER_FUNC(void, depth_test, bool32 enable);