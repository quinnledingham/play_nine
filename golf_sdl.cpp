#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "golf_defines.h"
#include "golf_types.h"
#include "golf_structs.h"
#include "golf_str.h"
#include "golf_sdl.h"
#include "golf_assets.h"
#include "golf_draw.h"
#include "golf_global.h"
#include "golf_input.h"
#include "golf_gui.h"

#include "golf_assets.cpp"
#include "golf_draw.cpp"
//#include "golf_gui.cpp"
#include "golf.cpp"

internal s32
sdl_init() {
  sdl_log("SDL!!!\n");

  bool init_result = SDL_Init(SDL_INIT_VIDEO);
  if (!init_result) {
    return -1;
  } else {
    sdl_log("Initialized SDL successfully.\n");
  }

  const int compiled = SDL_VERSION; // hardcoded number from SDL headers
  const int linked = SDL_GetVersion(); // reported by linked SDL library

  sdl_log("Compiled SDL version %d.%d.%d ...\n", SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
  sdl_log("Linked SDL version %d.%d.%d.\n", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

  sdl_log("Video Driver: %s\n", SDL_GetCurrentVideoDriver());

  // Create window
  const char *window_name = "golfo";
  int window_width = 800;
  int window_height = 500;
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE;
  sdl_ctx.window = SDL_CreateWindow(window_name, window_width, window_height, window_flags);
  if (!sdl_ctx.window) {
    sdl_log_error("%s\n", SDL_GetError());
    return FAILURE;
  } else {
    sdl_log("Window created successfully.\n");
  }

  sdl_log("Available Render Drivers:");
  int num_render_drivers = SDL_GetNumRenderDrivers();
  for (int i = 0; i < num_render_drivers; i++) {
    sdl_log("  %s", SDL_GetRenderDriver(i));
  }
  sdl_ctx.renderer = SDL_CreateRenderer(sdl_ctx.window, NULL);
  sdl_log("Renderer: %s\n", SDL_GetRendererName(sdl_ctx.renderer));

  if (!TTF_Init()) {
      sdl_log("Couldn't initialize SDL_ttf: %s\n", SDL_GetError());
      return SDL_APP_FAILURE;
  }

  init_buttons();

  sdl_ctx.text_engine = TTF_CreateRendererTextEngine(sdl_ctx.renderer);
  if (!sdl_ctx.text_engine) {
    sdl_log("Couldn't create text engine\n");
    return FAILURE;
  }

  load_assets(&fonts_info);

  return 0;
}

internal void
sdl_process_input() {
  app_input_set_previous_states();

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_EVENT_QUIT:
        sdl_ctx.should_quit = true;
        break;

      // Keyboard Events
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
        SDL_KeyboardEvent *keyboard_event = &event.key;

//#ifdef DEBUG
//        debug.last_key = keyboard_event->key;
//#endif // DEBUG

        for (u32 i = 0; i < ARRAY_COUNT(app_input.buttons); i++) {
          Button *button = &app_input.buttons[i];
          if (button_is_id(button, keyboard_event->key)) {
            button->current_state = keyboard_event->down;
          }
        }

      } break;
    }
  }
}

int main(int argc, char* argv[]) {
  sdl_init();


  Vector4 color = { 100, 60, 0, 255 };

  while(!sdl_ctx.should_quit) {
    sdl_process_input();

    update();

    if (on_down(IN_FORWARD)) {
      color = { 0, 60, 100, 255 };
    }

    SDL_SetRenderDrawColor(sdl_ctx.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(sdl_ctx.renderer);
    draw_text("LANA", {100, 100}, 18.0f, {0, 150, 0, 255});
    draw_text("LANA", {300, 100}, 18.0f, {0, 150, 0, 255});
    draw_text("LANA", {200, 100}, 18.0f, {0, 150, 0, 255});
    draw_text("LANA", {400, 100}, 18.0f, {0, 150, 0, 255});

    SDL_RenderPresent(sdl_ctx.renderer);
  }

  SDL_DestroyRenderer(sdl_ctx.renderer);
  SDL_DestroyWindow(sdl_ctx.window);
  TTF_Quit();
  SDL_Quit();

  return 0;
}
