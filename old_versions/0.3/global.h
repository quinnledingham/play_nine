struct App_Time {
  float64 run_time_s;
  float64 frame_time_s;
  float64 frames_per_s;
};

App_Time app_time;

struct SDL_Context {
  SDL_Window *window;
  bool8 should_quit;

  u64 start_ticks;
  u64 now_ticks;
  u64 last_ticks;
  u64 performance_frequency;

  bool8 relative_mouse_mode = false;

  // Cursors
  SDL_Cursor *pointer_cursor;
  SDL_Cursor *default_cursor;
};

SDL_Context sdl_ctx = {};

Vulkan_Context vk_ctx = {};

#define gfx vk_ctx

Scene scene;
Scene ortho_scene;

Camera camera = {};

Assets assets = {};

inline Pipeline* find_pipeline(u32 id) { return (Pipeline *)assets.pipelines.find(id); }
inline Texture*  find_texture (u32 id) { return (Texture *)assets.textures.find(id);   }
inline Font*     find_font    (u32 id) { return (Font *)assets.fonts.find(id);         }

/*
 Draw_Context contains meshes to do draw calls that are always useful like draw_rect or draw_text,
 in contrast to the draw calls to card meshes which are going to be defined in the assets for this
 game.
*/
struct Draw_Context {
  Mesh square;
  Mesh square_3D;
  Mesh rounded_rect;
  Mesh cube;
  Mesh sphere;

  u32 font_id;
};
Draw_Context draw_ctx = {};

Texture menu_noise;