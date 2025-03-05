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
struct GFX : Vulkan_Context {
  GFX_Window window;  
  GFX_Layout *layouts;

  bool8 vsync;
  bool8 anti_aliasing;
  bool8 resolution_scaling;

  u32 active_shader_id;

  void init();
  void destroy_frame_resources();
  void create_frame_resources();
  void default_viewport() { set_viewport(window.dim.width, window.dim.height); }
  void default_scissor() { set_scissor(0, 0, window.dim.width, window.dim.height); }
  Descriptor descriptor(u32 gfx_layout_id);

  void bind_pipeline(u32 id);
};

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