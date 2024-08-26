struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Object {
    Matrix_4x4 model;
    s32 index;
};

typedef Object GFX_Object;


Descriptor light_set;
Descriptor light_set_2;

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
RENDER_FUNC(void, create_graphics_pipeline, Shader *shader);
RENDER_FUNC(void, create_compute_pipeline, Shader *shader);
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
RENDER_FUNC(void, bind_descriptor_sets, Descriptor desc, void *data);
RENDER_FUNC(void, push_constants, u32 shader_stage, void *data, u32 data_size);
RENDER_FUNC(void, bind_descriptor_set, Descriptor desc);
RENDER_FUNC(void, update_ubo, Descriptor desc, void *data);
RENDER_FUNC(Descriptor, get_descriptor_set, u32 layout_id);
RENDER_FUNC(Descriptor, get_descriptor_set_index, u32 layout_id, u32 return_index);

#define GFX_FUNC(r, n, ...) r API3D_EXT(n)(__VA_ARGS__); r (*gfx_##n)(__VA_ARGS__) = &API3D_EXT(n)

GFX_FUNC(void, bind_shader, const char *tag);

void render_init_model(Model *model);

enum Resolution_Modes {
    RESOLUTION_480P,
    RESOLUTION_720P,
    RESOLUTION_1080P,
    RESOLUTION_2160P
};

struct Render {
    Vector2_s32 window_dim;
    bool8 vsync = FALSE;
    bool8 anti_aliasing = TRUE;

    // Resolution scaling
    Vector2_s32 resolution;
    u32 resolution_mode = RESOLUTION_2160P;
    bool8 resolution_scaling = false;
    float32 resolution_scale = 1.0f;

    void update_resolution();
    
    //bool8 depth_test
    u32 active_shader_id;
    bool8 recording_frame = FALSE;
    Layout *layouts;

    // Scissor
    u32 scissor_stack_index = 0;
    Rect scissor_stack[10];

    void scissor_push(Vector2 coords, Vector2 dim);
    void scissor_pop();
};


global Render render_context = {};

#define gfx render_context
#define GFX Render
