inline float64
get_seconds_elapsed(App_Time *time, u64 start, u64 end, u64 performance_frequency) {
  float64 result = ((float64)(end - start) / (float64)performance_frequency);
  return result;
}

void update_time(App_Time *time, u64 start, u64 last, u64 now, u64 performance_frequency) {
  // s
  time->frame_time_s = get_seconds_elapsed(time, last, now, performance_frequency);

  // time->start has to be initialized before
  time->run_time_s = get_seconds_elapsed(time, start, now, performance_frequency);

  // fps
  time->frames_per_s = 1.0 / time->frame_time_s;
}

void sdl_log(const char *msg, ...) {
  print_char_array(OUTPUT_DEFAULT, "(sdl) ");

  OUTPUT_LIST(OUTPUT_DEFAULT, msg);
}

void sdl_log_error(const char *msg, ...) {
  print_char_array(OUTPUT_ERROR, "ERROR: (sdl) ");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}

internal void
sdl_set_relative_mouse_mode(bool8 mode) {
  sdl_ctx.relative_mouse_mode = mode;
  SDL_SetWindowRelativeMouseMode(sdl_ctx.window, sdl_ctx.relative_mouse_mode);
}

internal void
sdl_toggle_relative_mouse_mode() {
  sdl_ctx.relative_mouse_mode = !sdl_ctx.relative_mouse_mode;
  SDL_SetWindowRelativeMouseMode(sdl_ctx.window, sdl_ctx.relative_mouse_mode);
}

internal void
sdl_set_icon() {
  SDL_Surface *icon_surface = SDL_LoadBMP("S:/play_nine/rewrite/assets/bitmaps/lana.bmp");
  if (icon_surface) {
    SDL_SetWindowIcon(sdl_ctx.window, icon_surface);
    SDL_DestroySurface(icon_surface);
  } else {
    ASSERT(0);
  }
}

internal void
sdl_loading_screen() {
  SDL_Renderer *renderer = SDL_CreateRenderer(sdl_ctx.window, "software");
  SDL_Surface  *loading_surface = SDL_LoadBMP("S:/play_nine/rewrite/assets/bitmaps/lana.bmp");
  if (renderer && loading_surface) {
    SDL_Texture *loading_texture = SDL_CreateTextureFromSurface(renderer, loading_surface);
    
    float32 ratio = (float)loading_texture->w / (float)loading_texture->h;

    SDL_FRect window_rect = { 0, 0, (float)gfx.window.dim.h * ratio, (float)gfx.window.dim.h };

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, loading_texture, NULL, &window_rect);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(loading_texture);
    SDL_DestroySurface(loading_surface);
    SDL_DestroyRenderer(renderer);
  } else {
    ASSERT(0);
  }
}

internal s32 
sdl_init() {
  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    sdl_log_error("(sdl) error: could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  const int compiled = SDL_VERSION; // hardcoded number from SDL headers
  const int linked = SDL_GetVersion(); // reported by linked SDL library

  sdl_log("Compiled SDL version %d.%d.%d ...\n", SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
  sdl_log("Linked SDL version %d.%d.%d.\n", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));
  
  u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  sdl_ctx.window = SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, sdl_window_flags);
  if (!sdl_ctx.window) {
    sdl_log_error("%s\n", SDL_GetError());
    return 1;
  }

  SDL_GetWindowSize(sdl_ctx.window, &gfx.window.dim.width, &gfx.window.dim.height);
  gfx.window.resolution = gfx.window.dim;

  // Log drivers that are available, in the order of priority SDL chooses them.
  // Useful for e.g. debugging which ones a particular build of SDL contains.
  sdl_log("Available renderer drivers:\n");
  for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
      sdl_log("%d. %s\n", i + 1, SDL_GetRenderDriver(i));
  }

  sdl_loading_screen();
  sdl_set_icon();
  sdl_ctx.start_ticks = SDL_GetPerformanceCounter();

  vulkan_sdl_init();
  vulkan_create_frame_resources();

  init_draw();
  play_init();

  SDL_srand(SDL_GetTicks());

  sdl_ctx.performance_frequency = SDL_GetPerformanceFrequency();

  sdl_ctx.pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
  sdl_ctx.default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);

  return 0;
}

s32 sdl_process_input() {

  app_input_set_previous_states();

  s32 return_val = 0;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        return_val = 1;
        break;

      case SDL_EVENT_WINDOW_MINIMIZED:
        gfx.window.minimized = true;
        break;
      case SDL_EVENT_WINDOW_RESTORED:
        gfx.window.minimized = false;
        break;

      case SDL_EVENT_WINDOW_RESIZED: {
        SDL_WindowEvent *window_event = &event.window;
        gfx.window.resized = true;
        gfx.window.dim.width  = window_event->data1;
        gfx.window.dim.height = window_event->data2;
        gfx.window.resolution = gfx.window.dim;
        //clay_set_layout((float)gfx.window.dim.width, (float)gfx.window.dim.height);
      } break;

      // Mouse Events
      case SDL_EVENT_MOUSE_MOTION:
        app_input.mouse.coords = { event.motion.x, event.motion.y };
        app_input.mouse.relative_coords += { event.motion.xrel, event.motion.yrel };
        app_input.last_input_type = IN_MOUSE;
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        app_input.mouse.coords = { event.button.x, event.button.y };
        sdl_log("MOUSE: %f, %f\n", app_input.mouse.coords.x, app_input.mouse.coords.y);

        if (event.button.button == SDL_BUTTON_LEFT) {
          app_input.mouse.left.current_state = TRUE;
        }
        app_input.last_input_type = IN_MOUSE;
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        app_input.mouse.coords = { event.button.x, event.button.y };
        if (event.button.button == SDL_BUTTON_LEFT) {
          app_input.mouse.left.current_state = FALSE;
        }
        app_input.last_input_type = IN_MOUSE;
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        //clay_update_scroll_containers(event.wheel.x, event.wheel.y);
        break;
      
      // Keyboard Events
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP: {
        SDL_KeyboardEvent *keyboard_event = &event.key;

#ifdef DEBUG
        debug.last_key = keyboard_event->key;
#endif // DEBUG

        for (u32 i = 0; i < ARRAY_COUNT(app_input.buttons); i++) {
          Button *button = &app_input.buttons[i];
          if (button_is_id(button, keyboard_event->key)) {
            button->current_state = keyboard_event->down;
          }
        }

      } break;
      
      default:
        break;
    }
  }

  //clay_set_pointer_state(mouse_coords.x, mouse_coords.y, left_mouse_button_down);

  return return_val;
}

s32 sdl_do_frame() {
  if (sdl_process_input())
    return 1;

  // frame time keeping
  sdl_ctx.last_ticks = sdl_ctx.now_ticks;
  sdl_ctx.now_ticks = SDL_GetPerformanceCounter();
  update_time(&app_time, sdl_ctx.start_ticks, sdl_ctx.last_ticks, sdl_ctx.now_ticks, sdl_ctx.performance_frequency);

  //app.update();
  //printf("%f\n", time.frames_per_s);

  if (do_game_frame() == FAILURE)
    return 1;

  return 0;
}

void sdl_cleanup() {
  SDL_DestroyCursor(sdl_ctx.pointer_cursor);
  SDL_DestroyCursor(sdl_ctx.default_cursor);

  SDL_DestroyWindow(sdl_ctx.window);
  SDL_Quit();
  TTF_Quit();
}

int main(int argc, char *argv[]) {
  init_output_buffer();
  sdl_log("starting sdl application...\n");
  sdl_log("CWD: %s\n", SDL_GetCurrentDirectory());
  
  sdl_init();

  while (1) {
    if (sdl_do_frame()) {
      break;
    }
  }

  vkDeviceWaitIdle(vk_ctx.device);
  play_destroy();
  assets_cleanup();
  vulkan_cleanup();
  sdl_cleanup();

  return 0;
}

internal void
sdl_wait_thread(SDL_Thread *thread) {
  int thread_return_value;
  if (NULL == thread) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s", SDL_GetError());
  } else {
      SDL_WaitThread(thread, &thread_return_value);
      SDL_Log("Thread returned value: %d", thread_return_value);
  }
}