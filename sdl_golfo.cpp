#include <SDL.h>
#include <SDL_main.h>

// Compiling to SPIR in code
//#ifdef DEBUG
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>
//#endif // DEBUG

#include <spirv_cross/spirv_cross_c.h>

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// Provided by VK_EXT_shader_object
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderEXT)

#define VK_USE_PLATFORM_WIN32_KHR

#include "defines.h"
#include "types.h"
#include "types_math.h"
#include "str.h"

#include "data_structs.h"
#include "vulkan_golfo.h"
#include "gfx.h"
#include "assets.h"
#include "input.h"

#include "global.h"
#include "golfo.h"
#include "golfo_assets.h"
#include "golfo_draw.h"
#include "gui.h"

#include <vector>

#include "vulkan_golfo.cpp"
#include "assets.cpp"
#include "golfo_draw.cpp"

internal void
sdl_set_relative_mouse_mode(bool8 mode) {
  sdl_ctx.relative_mouse_mode = mode;
  SDL_SetWindowRelativeMouseMode(sdl_ctx.window, sdl_ctx.relative_mouse_mode);
}

#include "gui.cpp"
#include "golfo_menus.cpp"

//#include "golfo_bitmaps.cpp"

internal s32
sdl_wait_thread(SDL_Thread *thread) {
  int thread_return_value;
  if (NULL == thread) {
      SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateThread failed: %s", SDL_GetError());
  } else {
      SDL_WaitThread(thread, &thread_return_value);
      SDL_Log("Thread returned value: %d", thread_return_value);
  }
  return thread_return_value;
}


#include "golfo.cpp"

internal s32
sdl_init() {
  SDL_Log("Initializing SDL...\n");
  
  // Basic initialization
  bool init_result = SDL_Init(SDL_INIT_VIDEO);
  if (!init_result) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL (%s)\n", SDL_GetError());
    return FAILURE;
  }

  const int compiled = SDL_VERSION; // hardcoded number from SDL headers
  const int linked = SDL_GetVersion(); // reported by linked SDL library

  SDL_Log("Compiled SDL version %d.%d.%d ...\n", SDL_VERSIONNUM_MAJOR(compiled), SDL_VERSIONNUM_MINOR(compiled), SDL_VERSIONNUM_MICRO(compiled));
  SDL_Log("Linked SDL version %d.%d.%d.\n", SDL_VERSIONNUM_MAJOR(linked), SDL_VERSIONNUM_MINOR(linked), SDL_VERSIONNUM_MICRO(linked));

  // Create window
  const char *window_name = "golfo";
  int window_width = 800;
  int window_height = 500;
  SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
  sdl_ctx.window = SDL_CreateWindow(window_name, window_width, window_height, window_flags);
  if (!sdl_ctx.window) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s\n", SDL_GetError());
    return FAILURE;
  }

  // Init vulkan
  vk_ctx.window_dim = { window_width, window_height };
  bool vulkan_init_result = vulkan_sdl_init();
  if (vulkan_init_result == FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize vulkan\n");
    return FAILURE;
  }

  vk_ctx.resolution = { window_width, window_height };

  vulkan_create_frame_resources();

  SDL_srand(SDL_GetTicks());
  
  golfo_init();

  sdl_ctx.pointer_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
  sdl_ctx.default_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);

  sdl_ctx.start_ticks = SDL_GetPerformanceCounter();
  sdl_ctx.performance_frequency = SDL_GetPerformanceFrequency();


  return SUCCESS;
}

internal void
sdl_process_input() {
  app_input.mouse.relative_coords = {};
  app_input.mouse.left.previous_state = app_input.mouse.left.current_state;

  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_EVENT_QUIT:
        sdl_ctx.should_quit = true;
        break;

      case SDL_EVENT_WINDOW_RESIZED: {
        SDL_WindowEvent *window_event = &event.window;
        vk_ctx.window_resized = true;
        vk_ctx.window_dim.width  = window_event->data1;
        vk_ctx.window_dim.height = window_event->data2;
        vk_ctx.resolution = vk_ctx.window_dim;
      } break;

      // Mouse Events
      case SDL_EVENT_MOUSE_MOTION:
        app_input.mouse.coords = { event.motion.x, event.motion.y };
        app_input.mouse.relative_coords += { event.motion.xrel, event.motion.yrel };
        app_input.last_input_type = IN_MOUSE;
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        app_input.mouse.coords = { event.button.x, event.button.y };
        SDL_Log("MOUSE: %f, %f\n", app_input.mouse.coords.x, app_input.mouse.coords.y);

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

    }
  }
}

inline float64
get_seconds_elapsed(App_Time *time, u64 start, u64 end, u64 performance_frequency) {
  float64 result = ((float64)(end - start) / (float64)performance_frequency);
  return result;
}

internal void 
update_time(App_Time *time, u64 start, u64 last, u64 now, u64 performance_frequency) {
  // s
  time->frame_time_s = get_seconds_elapsed(time, last, now, performance_frequency);

  // time->start has to be initialized before
  time->run_time_s = get_seconds_elapsed(time, start, now, performance_frequency);

  // fps
  time->frames_per_s = 1.0 / time->frame_time_s;
}

int main(int argc, char *argv[]) {
  if (sdl_init() == FAILURE) {
    return 1;
  }

  vulkan_clear_color({1, 1, 1, 1});

  while(1) {
    sdl_ctx.last_ticks = sdl_ctx.now_ticks;
    sdl_ctx.now_ticks = SDL_GetPerformanceCounter();
    update_time(&app_time, sdl_ctx.start_ticks, sdl_ctx.last_ticks, sdl_ctx.now_ticks, sdl_ctx.performance_frequency);  

    sdl_process_input();
    if (sdl_ctx.should_quit) {
      break;
    }

    if (vk_ctx.window_resized) {
      vulkan_destroy_frame_resources();
      vulkan_create_frame_resources();
      vk_ctx.window_resized = false;
    }

    if (golfo_update()) {
      break;
    }
  }
  
  return 0;
}
