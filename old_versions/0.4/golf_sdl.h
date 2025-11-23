struct SDL_Context {
  bool8 should_quit;
  SDL_Window *window;
  SDL_Renderer *renderer;
  TTF_TextEngine *text_engine;
};

#define sdl_log(fmt, ...) SDL_Log("(sdl) " fmt, ##__VA_ARGS__)
#define sdl_log_error(fmt, ...) SDL_LogError(SDL_LOG_CATEGORY_ERROR, "(sdl) " fmt, ##__VA_ARGS__)
