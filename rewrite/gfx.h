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

enum {
    GFX_DO_FRAME,
    GFX_SKIP_FRAME,
    GFX_ERROR,

    GFX_COUNT
};

/*
Mainly inheriting the functions from the context
*/
struct GFX {
  GFX_Window window;  
  GFX_Layout *layouts;
  u32 layouts_count;

  bool8 vsync = false;
  bool8 anti_aliasing = true;
  bool8 resolution_scaling;

  bool8 enforce_frame_rate = true;
  u32 target_frame_rate = 60;

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

/*
  Text:
    bind texture (font)/color, use text coloring
    bind texcoords, position

  Model:
    per mesh
        bind texture/color, with to use text or color, texcoords, position

    custom
        bind texture/color (card texture and color)
        bind use text or color, texcoords, position
*/

//
// Drawing
//

struct Global_Shader {
    Vector4 resolution; // rect
    Vector4 time; // time, time_delta, frame_rate
};

struct Scene {
    Matrix_4x4 view;
    Matrix_4x4 projection;
};

struct Texture_Region {
    Vector2 uv_offset;
    Vector2 uv_scale;
};

struct Local {
    Vector4 text;
    Vector4 color;
    Texture_Region region;
};

internal void gfx_bind_descriptor_set(Local *local);

struct Object {
    Matrix_4x4 model;
    s32 index;
};

enum Interpolation_Types {
    INTERP_LERP,
    INTERP_SLERP,
    INTERP_FUNC,

    INTERP_COUNT
};

struct Animation_Keyframe {
    Transform start;
    Transform end;

    float32 time_elapsed;
    float32 time_duration;

    u32 interpolation; // lerp, slerp

    bool8 dynamic;

    // if dynamic == false
    // allows for a moving destination.
    Transform *dest;

    void *func_args;
    bool8 (*func)(void *args);
};

struct Animation {
    bool8 active;
    Transform *src; // points to a the pose that is being animated

    Array<Animation_Keyframe> keyframes;
};

struct Camera {
    // defines camera
    union {
        struct {
            Vector3 position;
            union {
                struct {
                    float32 roll, pitch, yaw;
                };
                Vector3 orientation;
            };
        };
        Pose pose;
    };

    // initialized by pose
    Vector3 right;
    Vector3 up;
    Vector3 direction;

    float32 fov;

    //Animation *animation;
};