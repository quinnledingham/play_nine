/*
 Only every going to have one SDL_Context or one Vulkan_Context in this game
 so it makes sense to just have them be global.

 The goal is to complete this game, not make a generic engine. To complete this
 game I am going to use SDL and Vulkan... so don't try to generalize for other
 platforms or apis.
*/

SDL_Context sdl_ctx;
Vulkan_Context vk_ctx;
GFX gfx = {};
Assets assets = {};

/*
 Draw_Context contains meshes to do draw calls that are always useful like draw_rect or draw_text,
 in contrast to the draw calls to card meshes which are going to be defined in the assets for this
 game.
*/
struct Draw_Context {
  Mesh square;
};
Draw_Context draw_context = {};

SDL_Renderer_Context sdl_renderer_context;

Shader test = {};

Scene scene;
Scene ortho_scene;

inline Pipeline* 
find_pipeline(u32 id) {
  return ((Pipeline *)assets.pipelines.buffer.memory) + id;
}

inline Font* 
find_font(u32 id) {
  return ((Font *)assets.fonts.buffer.memory) + id;
}

bool8 local_vulkan_context = false;

// Play Nine Game

s8 deck[DECK_SIZE];
Game test_game;

// Drawing Game

Vector2 hand_coords[HAND_SIZE];
float32 hand_width;
Vector2 card_dim = { 20.0f, 32.0f };
