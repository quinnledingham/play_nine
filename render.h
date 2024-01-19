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
    
    //VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;

    void *gpu_info; // OpenGL = NULL, Vulkan = VkPipeline
};

struct Render {

};

global Render render_info = {};

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
RENDER_FUNC(void, cleanup, );
RENDER_FUNC(void, create_graphics_pipeline, Render_Pipeline *pipeline);
RENDER_FUNC(void, bind_pipeline, Render_Pipeline *pipeline);

RENDER_FUNC(void, create_descriptor_pool, Shader *shader, u32 descriptor_set_count, u32 set_index);
RENDER_FUNC(Descriptor_Set*, get_descriptor_set, Shader *shader, bool8 layout_index);
RENDER_FUNC(void, init_bitmap, Descriptor_Set *set, Bitmap *bitmap, u32 binding);
RENDER_FUNC(void, update_ubo, Descriptor_Set *set, u32 descriptor_index, void *data, bool8 static_update);

// this is when you tell the shader where the memory is
RENDER_FUNC(void, bind_descriptor_set, Descriptor_Set *set, u32 first_set);

RENDER_FUNC(void, init_mesh, Mesh *mesh);
RENDER_FUNC(void, draw_mesh, Mesh *mesh);
RENDER_FUNC(void, set_viewport, u32 window_width, u32 window_height);
RENDER_FUNC(void, set_scissor, u32 window_width, u32 window_height);

RENDER_FUNC(void, compile_shader, Shader *shader);