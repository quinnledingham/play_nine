struct Rect {
    union {
        struct {
            float32 x;
            float32 y;
        };
        Vector2 coords;
    };

    union {
        struct {
            float32 width;
            float32 height;
        };
        struct {
            float32 w;
            float32 h;
        };
        Vector2 dim;
    };
    float32 rotation; // radians
};

//
// Main GFX
//

struct GFX_Window {
  Vector2_s32 dim;
  Vector2_s32 resolution;
  float32 aspect_ratio;
  bool8 minimized;
  bool8 resized;
};

/*
Mainly inheriting the functions from the context
*/
struct GFX {
  GFX_Window window;  
  GFX_Layout *layouts;
  u32 layouts_count;

  bool8 vsync;
  bool8 anti_aliasing = true;
  bool8 resolution_scaling;

  Stack<Rect> scissor_stack = Stack<Rect>(10);

  u32 active_shader_id;
};

internal void gfx_define_layouts();
internal void gfx_add_layouts_to_shaders();
inline Descriptor_Set gfx_descriptor_set(u32 gfx_layout_id);
inline Descriptor gfx_descriptor(Descriptor_Set *set, u32 binding_index);


internal void gfx_bind_bitmap(u32 gfx_id, Bitmap *bitmap, u32 binding);
internal void gfx_bind_bitmap(u32 gfx_id, u32 bitmap_id, u32 binding);
internal void gfx_bind_descriptor_set(u32 gfx_id, void *data);

// used with render_create_texture function
enum Texture_Parameters {
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

//
// Drawing
//

struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Local {
    Vector4 text;
    Vector4 color;
    Vector4 resolution; // rect
    Vector4 time; // time, time_delta, frame_rate
};

internal void gfx_bind_descriptor_set(Local *local);

struct Object {
    Matrix_4x4 model;
    s32 index;
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