struct App_Time {
  float64 run_time_s;
  float64 frame_time_s;
  float64 frames_per_s;
};

struct SDL_Context {
  SDL_Window *window;
  SDL_Renderer *renderer; // Clay

  u64 start_ticks;
  u64 now_ticks;
  u64 last_ticks;
  u64 performance_frequency;

  bool8 relative_mouse_mode = false;
  
  SDL_Cursor *pointer_cursor;
  SDL_Cursor *default_cursor;
};

struct SDL_Renderer_Context {
  SDL_Renderer *renderer;
  TTF_TextEngine *text_engine;

  bool8 swap_chain_created;
};

internal void sdl_toggle_relative_mouse_mode();