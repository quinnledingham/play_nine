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
App_Input app_input = {};

enum GUI_Ids {
  GUI_MAIN_MENU,
  GUI_TEST,
  GUI_PAUSE,

  GUI_COUNT
};

struct GUI_Manager {
  GUI guis[GUI_COUNT];

  Stack<u32> indices = Stack<u32>(10); // index at the top is the gui that is currently rendered
};

GUI_Manager gui_manager;

inline s32
draw_top_gui() {
  // Drawing no menu... if esc is pressed add pause to the stack
  if (gui_manager.indices.empty()) {
    if (on_down(app_input.back)) {
      sdl_toggle_relative_mouse_mode();
      gui_manager.indices.push(GUI_PAUSE);
    }

    return SUCCESS;
  }

  u32 index = gui_manager.indices.top();
  GUI *gui = &gui_manager.guis[index];
#ifdef DEBUG
  if (!gui->draw) {
    log_error("draw_top_gui(): draw function not set for index (%d)\n", index);
    return FAILURE;
  }
#endif // DEBUG
  return gui->draw(gui);
}

inline Pipeline* 
find_pipeline(u32 id) {
  return (Pipeline *)assets.pipelines.find(id);
}

inline Font* 
find_font(u32 id) {
  return ((Font *)assets.fonts.buffer.memory) + id;
}

inline Bitmap* 
find_bitmap(u32 id) {
  return ((Bitmap *)assets.bitmaps.buffer.memory) + id;
}

inline Texture_Atlas* 
find_atlas(u32 id) {
  return ((Texture_Atlas *)assets.atlases.buffer.memory) + id;
}

inline Material_Library* 
find_mtllib(u32 id) {
  return ((Material_Library *)assets.mtllibs.buffer.memory) + id;
}

inline Geometry* 
find_geometry(u32 id) {
  return ((Geometry *)assets.geometrys.buffer.memory) + id;
}

u32 last_key = SDLK_A;

/*
 Draw_Context contains meshes to do draw calls that are always useful like draw_rect or draw_text,
 in contrast to the draw calls to card meshes which are going to be defined in the assets for this
 game.
*/
struct Draw_Context {
  Mesh square;
  Mesh square_3D;
  Mesh rounded_rect;

  u32 font_id;
};
Draw_Context draw_ctx = {};

Buffer global_buffer = blank_buffer(20);

// Play Nine Game

s8 deck[DECK_SIZE];
Game test_game;

Game game;
Game_Draw game_draw;

// Drawing Game

bool8 draw_game_flag = false;

Bitmap card_bitmaps[14];
Texture_Atlas card_bitmaps_atlas;

Camera camera = {
  Vector3{0, 0, 1},
  Vector3{0, 0, 0},
  Vector3{0, -1, 0},
  0, 
  0, 
  0
};

Scene scene;
Scene ortho_scene;


