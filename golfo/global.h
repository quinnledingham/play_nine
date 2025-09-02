struct SDL_Context {
  SDL_Window *window;
  bool8 should_quit;
};

SDL_Context sdl_ctx = {};

Vulkan_Context vk_ctx = {};

#define gfx vk_ctx

Scene scene;
Scene ortho_scene;

Camera camera = {};

Assets assets = {};

inline Pipeline* find_pipeline(u32 id) { return (Pipeline *)assets.pipelines.find(id); }
inline Texture* find_texture(u32 id) { return (Texture *)assets.textures.find(id); }

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