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
App_Time app_time;

inline Pipeline* 
find_pipeline(u32 id) {
  return ((Pipeline *)assets.pipelines.buffer.memory) + id;
}

inline Font* 
find_font(u32 id) {
  return ((Font *)assets.fonts.buffer.memory) + id;
}

inline Bitmap* 
find_bitmap(u32 id) {
  return ((Bitmap *)assets.bitmaps.buffer.memory) + id;
}

/*
 Draw_Context contains meshes to do draw calls that are always useful like draw_rect or draw_text,
 in contrast to the draw calls to card meshes which are going to be defined in the assets for this
 game.
*/
struct Draw_Context {
  Mesh square;

  u32 font_id;
};
Draw_Context draw_ctx = {};

// Play Nine Game

s8 deck[DECK_SIZE];
Game test_game;

// Drawing Game

Scene scene;
Scene ortho_scene;

Vector2 hand_coords[HAND_SIZE];
float32 hand_width;
Vector2 card_dim = { 20.0f, 32.0f };

global const Vector4 play_nine_green        = {  39,  77,  20, 1 };
global const Vector4 play_nine_yellow       = { 231, 213,  36, 1 };
global const Vector4 play_nine_light_yellow = { 240, 229, 118, 1 };
global const Vector4 play_nine_dark_yellow  = { 197, 180,  22, 1 };