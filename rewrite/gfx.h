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

  u32 active_shader_id;
};

void gfx_init();

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

struct Object {
    Matrix_4x4 model;
    s32 index;
};
