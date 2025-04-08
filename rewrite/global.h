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

GUI_Manager gui_manager;

inline Pipeline* find_pipeline(u32 id) { return (Pipeline *)assets.pipelines.find(id); }
inline Font* find_font(u32 id) { return ((Font *)assets.fonts.buffer.memory) + id; }
inline Bitmap* find_bitmap(u32 id) { return ((Bitmap *)assets.bitmaps.buffer.memory) + id; }
inline Texture_Atlas*  find_atlas(u32 id) { return ((Texture_Atlas *)assets.atlases.buffer.memory) + id; }
inline Material_Library* find_mtllib(u32 id) { return ((Material_Library *)assets.mtllibs.buffer.memory) + id; }
inline Geometry* find_geometry(u32 id) { return ((Geometry *)assets.geometrys.buffer.memory) + id; }

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

Buffer global_buffer = blank_buffer(20);

// Play Nine Game

s8 deck[DECK_SIZE];

Game game;
Game_Draw game_draw;

enum Game_Type {
  LOCAL_GAME,
  ONLINE_GAME,
};
u32 game_type = LOCAL_GAME;

// Drawing Game

Texture_Atlas card_bitmaps_atlas;

Camera camera = {};
Ray mouse_ray = {};

Scene scene;
Scene ortho_scene;

enum Entity_Types {
  ET_CARD,

  ET_COUNT
};

Array<Animation> animations;

#ifdef DEBUG

struct Debug_State {
  bool8 draw_text_info = false;
  bool8 free_camera = false;

  bool8 recreate_input_prompt_atlases = false;
  bool8 recreate_bitmaps = false;

  u32 last_key = SDLK_A;
  Bitmap resized_lana = {};
  Game test_game;

  bool8 load_assets = true;
  FILE *asset_save_file = fopen("assets.ethan", "wb");
  FILE *asset_load_file = fopen("assets.ethan", "rb");

  // GFX
  bool8 wireframe;
};

Debug_State debug = {};

#endif // DEBUG