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
  SDL_version version;
  SDL_Window *window;

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
  print_char_array(OUTPUT_ERROR, "ERROR: (sdl)");

  OUTPUT_LIST(OUTPUT_ERROR, msg);
}

s32 SDL_Context::init() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    sdl_log_error("(sdl) error: could not initialize SDL: %s\n", SDL_GetError());
    return 1;
  }

  SDL_GetVersion(&version);
  sdl_log("(sdl) version: %d.%d.%d\n", version.major, version.minor, version.patch);
  
  u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  window = SDL_CreateWindow(WINDOW_NAME, 
                            SDL_WINDOWPOS_UNDEFINED, 
                            SDL_WINDOWPOS_UNDEFINED, 
                            WINDOW_WIDTH, 
                            WINDOW_HEIGHT, 
                            sdl_window_flags);
  if (!window) {
    sdl_log_error("%s\n", SDL_GetError());
    return 1;
  }

  gfx.sdl_init(window);
  //gfx.init();
  SDL_GetWindowSize(window, &gfx.window.dim.width, &gfx.window.dim.height);
  gfx.window.resolution = gfx.window.dim;
  gfx.create_frame_resources();

  gfx.clear_color({1, 0, 1, 1});

  start_ticks = SDL_GetPerformanceCounter();

  return 0;
}


s32 SDL_Context::window_event(SDL_WindowEvent *window_event) {
  switch(window_event->event) {
    //case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED: {
        gfx.window.resized = true;
        gfx.window.dim.width  = window_event->data1;
        gfx.window.dim.height = window_event->data2;
        gfx.window.resolution = gfx.window.dim;
    } break;

    case SDL_WINDOWEVENT_MOVED: {
    } break;
/*

    case SDL_WINDOWEVENT_MINIMIZED: {
        app->window.minimized = true;
    } break;

    case SDL_WINDOWEVENT_RESTORED: {
        app->window.minimized = false;
    } break;
*/
  }

  return 0;
}

s32 SDL_Context::process_input() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: {
        return 1;
      } break;

      case SDL_WINDOWEVENT: {
        window_event(&event.window);
      } break;
    }
  }

  return 0;
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
    
  update();

  return 0;
}

void SDL_Context::cleanup() {
  SDL_DestroyWindow(window);
  SDL_Quit();
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

  if (init()) {
    log_error("init(): failed\n");
    return 1;
  }

  while (1) {
    if (sdl_context.do_frame()) {
      break;
    }
  }

  gfx.cleanup();
  sdl_context.cleanup();

  return 0;
}
