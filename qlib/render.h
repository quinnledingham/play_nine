struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    Matrix_4x4 model;
    s32 index;
};

typedef Object GFX_Object;

struct Render_Pipeline {
    Shader *shader;
    bool8 compute = FALSE;
    bool8 blend;
    bool8 depth_test = TRUE;
    bool8 wireframe;

#ifdef VULKAN
    VkPipelineLayout pipeline_layout;
    VkPipeline graphics_pipeline;
#endif

    void *gpu_info; // OpenGL = NULL, Vulkan = VkPipeline
};

enum Pipelines {
    PIPELINE_2D_COLOR,
    PIPELINE_2D_TEXTURE,
    PIPELINE_2D_TEXT,

    PIPELINE_3D_COLOR,
    PIPELINE_3D_TEXTURE,
    PIPELINE_3D_TEXT,

    PIPELINE_RAY,
    PIPELINE_PROMPT,

    PIPELINE_COUNT
};

Render_Pipeline pipelines[PIPELINE_COUNT];

Descriptor light_set;
Descriptor light_set_2;
Layout layouts[11];

#define TEXTURE_ARRAY_SIZE   16
#define MAX_FRAMES_IN_FLIGHT  2

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

// create identifier and then set function pointer
// t = api, n = name, a == args
#define RENDER_FUNC(r, n, ...) r API3D_EXT(n)(__VA_ARGS__); r (*render_##n)(__VA_ARGS__) = &API3D_EXT(n)

RENDER_FUNC(bool8, sdl_init, SDL_Window *sdl_window);
RENDER_FUNC(void, clear_color, Vector4 color);
RENDER_FUNC(bool8, start_frame, App_Window *window);
RENDER_FUNC(void, end_frame, Assets *assets, App_Window *window);
RENDER_FUNC(void, cleanup);
RENDER_FUNC(void, create_graphics_pipeline, Render_Pipeline *pipeline, Vertex_Info vertex_info);
RENDER_FUNC(void, create_compute_pipeline, Render_Pipeline *pipeline);
RENDER_FUNC(void, pipeline_cleanup, Render_Pipeline *pipe);
RENDER_FUNC(void, bind_pipeline, Render_Pipeline *pipeline);
RENDER_FUNC(void, create_texture, Bitmap *bitmap, u32 texture_parameters);
RENDER_FUNC(void, delete_texture, Bitmap *bitmap);
RENDER_FUNC(u32, set_bitmap, Descriptor *desc, Bitmap *bitmap);
RENDER_FUNC(void, init_mesh, Mesh *mesh);
RENDER_FUNC(void, draw_mesh, Mesh *mesh);
RENDER_FUNC(void, set_viewport, u32 window_width, u32 window_height);
RENDER_FUNC(void, set_scissor, s32 x, s32 y, u32 window_width, u32 window_height);
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

void render_init_model(Model *model);

enum Resolution_Modes {
    RESOLUTION_480P,
    RESOLUTION_720P,
    RESOLUTION_1080P,
    RESOLUTION_2160P
};

internal float32 
get_resolution_scale(u32 resolution_mode) {
    float32 resolution_scales[4] = {
        0.25f,
        0.5f,
        0.75f,
        1.0f
    };

    return resolution_scales[resolution_mode];
}

internal Vector2_s32
get_resolution(Vector2_s32 in_resolution, float32 scale) {
    Vector2 new_resolution = cv2(in_resolution) * scale;
    Vector2_s32 out_resolution = { s32(new_resolution.x), s32(new_resolution.y) };
    return out_resolution;
}

struct Render {
    Vector2_s32 window_dim;
    Vector2_s32 resolution;
    bool8 vsync = FALSE;
    bool8 anti_aliasing = TRUE;
    
    u32 resolution_mode = RESOLUTION_2160P;
    bool8 resolution_scaling = false;
    float32 resolution_scale = 1.0f;

    void update_resolution() {
        resolution = get_resolution(window_dim, resolution_scale);

        if (resolution == window_dim) {
            resolution_scaling = FALSE;
        } else {
            resolution_scaling = TRUE;
        }
    }
    
    //bool8 depth_test;

    u32 scissor_stack_index = 0;
    Rect scissor_stack[10];

    void scissor_push(Vector2 coords, Vector2 dim) {
        Vector2 factor = {
            (float32)resolution.x / (float32)window_dim.x,
            (float32)resolution.y / (float32)window_dim.y
        };
        
        Rect rect = {};
        rect.coords = coords * factor;
        rect.dim = dim * factor;
        scissor_stack[scissor_stack_index++] = rect;
        render_set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (s32)rect.dim.x, (s32)rect.dim.y);
    }

    void scissor_pop();
/*
    void scissor_pop() {
        scissor_stack_index--;
        Rect rect = scissor_stack[scissor_stack_index - 1];
        render_set_scissor((s32)rect.coords.x, (s32)rect.coords.y, (u32)rect.dim.x, (u32)rect.dim.y);
    }
*/
};


global Render render_context = {};

#define gfx render_context

struct GFX {

};
