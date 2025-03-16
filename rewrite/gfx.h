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

struct GFX_Function_Table {
  void (*start_frame)();
  void (*end_frame)();
  void (*draw_mesh)(Mesh *mesh);
};

struct Draw_Function_Table {
  void (*draw_start)();
  void (*draw_end)();
  void (*draw_rect)(Vector2 coords, Vector2 size, Vector4 color);
  void (*draw_text)(Vector2 coords, const char *string, Vector4 color, u32 font_id);
};

/*
Mainly inheriting the functions from the context
*/
struct GFX {
  GFX_Window window;  
  GFX_Layout *layouts;

  bool8 vsync;
  bool8 anti_aliasing;
  bool8 resolution_scaling;

  u32 active_shader_id;
  
  GFX_Function_Table func;

  Vulkan_Context *vk_ctx;

  Vulkan_Context vulkan_context;

  void init();
  void default_viewport();
  void default_scissor();
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
