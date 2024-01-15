struct Uniform_Buffer_Object {
    void *handle; // OpenGL = u32; Vulkan = void*
    u32 size;
    u32 offsets[2];

    u32 opengl() {
        return *(u32*)handle;
    }
};

struct Scene {
    Matrix_4x4 model;
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    
};

struct Render_Pipeline {
    Shader shader;
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
RENDER_FUNC(void, update_uniform_buffer_object, Uniform_Buffer_Object *ubo, void *data, u32 data_size);
RENDER_FUNC(void, init_mesh, Mesh *mesh);
RENDER_FUNC(void, draw_mesh, Mesh *mesh);