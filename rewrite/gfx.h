enum GFX_Display_Mode {
  DISPLAY_MODE_WINDOWED,
  DISPLAY_MODE_FULLSCREEN,
  DISPLAY_MODE_WINDOWED_FULLSCREEN,

  DISPLAY_MODE_COUNT
};

struct GFX_Window {
  Vector2_s32 dim;
  float32 aspect_ratio;
  bool8 minimized;
  GFX_Display_Mode display_mode;
};

struct GFX {
  GFX_Window window;
  s32 (*init)(SDL_Window *sdl_window);
};

GFX gfx = {
  .init = vulkan_sdl_init,
};

