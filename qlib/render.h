struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    Matrix_4x4 model;
    s32 index;
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
    bool8 depth_test;
};

global Render render_info = {};
Descriptor light_set;
Descriptor light_set_2;
Layout layouts[10];

#define TEXTURE_ARRAY_SIZE 64
#define MAX_FRAMES_IN_FLIGHT 2

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

RENDER_FUNC(void, create_texture, Bitmap *bitmap, u32 texture_parameters);
RENDER_FUNC(void, delete_texture, Bitmap *bitmap);
RENDER_FUNC(u32, set_bitmap, Descriptor *desc, Bitmap *bitmap);

RENDER_FUNC(void, init_mesh, Mesh *mesh);
RENDER_FUNC(void, draw_mesh, Mesh *mesh);
void render_init_model(Model *model);

RENDER_FUNC(void, set_viewport, u32 window_width, u32 window_height);
RENDER_FUNC(void, set_scissor, s32 x, s32 y, u32 window_width, u32 window_height);

RENDER_FUNC(void, compile_shader, Shader *shader);

RENDER_FUNC(void, assets_cleanup, Assets *assets);
RENDER_FUNC(void, wait_frame, );

RENDER_FUNC(void, depth_test, bool32 enable);

RENDER_FUNC(void, create_set_layout, Layout *layout);
RENDER_FUNC(void, allocate_descriptor_set, Layout *layout);

RENDER_FUNC(void, init_layout_offsets, Layout *layout, Bitmap *bitmap);
RENDER_FUNC(void, bind_descriptor_sets, Descriptor desc, u32 first_set, void *data, u32 size);
RENDER_FUNC(void, push_constants, u32 shader_stage, void *data, u32 data_size);
RENDER_FUNC(void, bind_descriptor_set, Descriptor desc);
RENDER_FUNC(void, update_ubo, Descriptor desc, void *data);
RENDER_FUNC(Descriptor, get_descriptor_set, Layout *layout);
RENDER_FUNC(Descriptor, get_descriptor_set_index, Layout *layout, u32 return_index);