void sdl_update_time(App_Time *time) {
  s64 ticks = SDL_GetPerformanceCounter();

  // s
  time->frame_time_s = get_seconds_elapsed(time, time->last_frame_ticks, ticks);
  time->last_frame_ticks = ticks; // set last ticks for next frame

  // time->start has to be initialized before
  time->run_time_s = get_seconds_elapsed(time, time->start_ticks, ticks);

  // fps
  time->frames_per_s = 1.0 / time->frame_time_s;
}

bool8 sdl_process_input() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT: {
        return true;
      } break;
    }
  }

  return false;
}

int main(int argc, char *argv[]) {
  printf("(sdl) starting sdl application...\n");

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "(sdl) error: could not initialize SDL: %s\n", SDL_GetError());
  }
  
  SDL_version sdl_version = {};
  SDL_GetVersion(&sdl_version);
  printf("(sdl) version: %d.%d.%d\n", sdl_version.major, sdl_version.minor, sdl_version.patch);

  u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  SDL_Window *sdl_window = SDL_CreateWindow(WINDOW_NAME, 
                                            SDL_WINDOWPOS_UNDEFINED, 
                                            SDL_WINDOWPOS_UNDEFINED, 
                                            WINDOW_WIDTH, 
                                            WINDOW_HEIGHT, 
                                            sdl_window_flags);
  if (!sdl_window) {
    fprintf(stderr, "%s\n", SDL_GetError());
    return 1;
  }

  if (gfx.init(sdl_window)) {
    fprintf(stderr, "(sdl) couldn't init gfx\n");
    return 1;
  }

  app.time.performance_frequency = SDL_GetPerformanceFrequency();
  app.time.start_ticks           = SDL_GetPerformanceCounter();
  app.time.last_frame_ticks      = app.time.start_ticks;

  event_handler(EVENT_INIT);

  if (app.update == 0) {
    printf("(sdl) main(): no update function set\n");
    return 1;
  }

  while (1) {
    if (sdl_process_input())
      break;

    sdl_update_time(&app.time);

    app.update();
  }

  event_handler(EVENT_QUIT);

  SDL_DestroyWindow(sdl_window);

  return 0;
}
