struct App_Time {
  float64 run_time_s;
  float64 frame_time_s;
  float64 frames_per_s;
};

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

struct SDL_Context {
  SDL_Window *window;

  SDL_Renderer *renderer; // Clay

  u64 start_ticks;
  u64 now_ticks;
  u64 last_ticks;

  // Functions
  s32 do_frame();
  s32 process_input();
  s32 init();
  void cleanup();
  s32 window_event(SDL_WindowEvent *window_event);
};

void sdl_log(const char *msg, ...) {
  print_char_array(OUTPUT_DEFAULT, "(sdl) ");

  OUTPUT_LIST(OUTPUT_DEFAULT, msg);
}

void sdl_log_error(const char *msg, ...) {
  print_char_array(OUTPUT_ERROR, "ERROR: (sdl) ");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}

s32 SDL_Context::init() {

  if (!TTF_Init()) {
      return 1;
  }

  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "vulkan");

  if (SDL_Init(SDL_INIT_VIDEO) == false) {
    sdl_log_error("(sdl) error: could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  const int compiled = SDL_VERSION; // hardcoded number from SDL headers
  const int linked = SDL_GetVersion(); // reported by linked SDL library

  sdl_log("We compiled against SDL version %d.%d.%d ...\n", SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
  sdl_log("But we are linking against SDL version %d.%d.%d.\n", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));
  
  u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  window = SDL_CreateWindow(WINDOW_NAME, WINDOW_WIDTH, WINDOW_HEIGHT, sdl_window_flags);
  if (!window) {
    sdl_log_error("%s\n", SDL_GetError());
    return 1;
  }

  // Log drivers that are available, in the order of priority SDL chooses them.
  // Useful for e.g. debugging which ones a particular build of SDL contains.
  sdl_log("Available renderer drivers:\n");
  for (int i = 0; i < SDL_GetNumRenderDrivers(); i++) {
      sdl_log("%d. %s\n", i + 1, SDL_GetRenderDriver(i));
  }

  renderer = SDL_CreateRenderer(window, NULL);

  SDL_GetWindowSize(window, &gfx.window.dim.width, &gfx.window.dim.height);
  gfx.window.resolution = gfx.window.dim;

  gfx.sdl_init(window);
  
  //gfx.create_frame_resources();

  start_ticks = SDL_GetPerformanceCounter();
/*
  if (play_nine_init()) {
    log_error("init(): failed\n");
    return 1;
  }
*/
  init_clay(renderer, (float)gfx.window.dim.width, (float)gfx.window.dim.height);

  return 0;
}

bool8 left_mouse_button_down = false;
Vector2 mouse_coords = {};

s32 SDL_Context::process_input() {

  s32 return_val = 0;

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_QUIT:
        return_val = 1;
        break;

      case SDL_EVENT_WINDOW_RESIZED: {
        SDL_WindowEvent *window_event = &event.window;
        gfx.window.resized = true;
        gfx.window.dim.width  = window_event->data1;
        gfx.window.dim.height = window_event->data2;
        gfx.window.resolution = gfx.window.dim;
        clay_set_layout((float)gfx.window.dim.width, (float)gfx.window.dim.height);
      } break;
      case SDL_EVENT_MOUSE_MOTION:
        mouse_coords = { event.motion.x, event.motion.y };
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        mouse_coords = { event.button.x, event.button.y };
        if (event.button.button == SDL_BUTTON_LEFT)
          left_mouse_button_down = true;
        break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
        mouse_coords = { event.button.x, event.button.y };
        if (event.button.button == SDL_BUTTON_LEFT) {
          left_mouse_button_down = false;
        }
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        clay_update_scroll_containers(event.wheel.x, event.wheel.y);
        break;
      case SDL_EVENT_KEY_DOWN:
        if (!local_vulkan_context) {
          local_vulkan_context = true;
          SDL_DestroyRenderer(renderer);
          gfx.create_frame_resources();
          play_nine_init();
        } else {
          local_vulkan_context = false;
          gfx.destroy_frame_resources();
          renderer = SDL_CreateRenderer(window, NULL);
          if (!renderer) {
            sdl_log_error("Failed to create renderer\n");
          }
          clay_set_renderer(renderer);
        }
        break;
      default:
        break;
    }
  }

  clay_set_pointer_state(mouse_coords.x, mouse_coords.y, left_mouse_button_down);

  return return_val;
}

s32 SDL_Context::do_frame() {
  if (process_input())
    return 1;

  // frame time keeping
  App_Time time;
  last_ticks = now_ticks;
  now_ticks = SDL_GetPerformanceCounter();
  u64 performance_frequency = SDL_GetPerformanceFrequency();
  update_time(&time, start_ticks, last_ticks, now_ticks, performance_frequency);

  //app.update();
  //printf("%f\n", time.frames_per_s);

  if (local_vulkan_context)
    update();
  else 
    draw_clay();

  return 0;
}

void SDL_Context::cleanup() {
  SDL_DestroyWindow(window);
  SDL_Quit();
  TTF_Quit();
}

int main(int argc, char *argv[]) {
  init_output_buffer();
  sdl_log("starting sdl application...\n");

  {
    String test = String("Test.lmao");
    test.remove_ending();
    test.log();
  }

  SDL_Context sdl_context = {};
  
  sdl_context.init();

  while (1) {
    if (sdl_context.do_frame()) {
      break;
    }
  }

  gfx.cleanup();
  sdl_context.cleanup();

  return 0;
}
