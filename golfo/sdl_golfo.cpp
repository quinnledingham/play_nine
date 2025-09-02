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
#include "gfx.h"
#include "assets.h"

#include "vulkan_golfo.h"
#include "global.h"
#include "golfo_assets.h"

#include <vector>

#include "vulkan_golfo.cpp"
#include "assets.cpp"
#include "golfo_draw.cpp"
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
  vk_ctx.window_dim = { (u32)window_width, (u32)window_height };
  bool vulkan_init_result = vulkan_sdl_init();
  if (vulkan_init_result == FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize vulkan\n");
    return FAILURE;
  }

  vk_ctx.resolution = { (u32)window_width, (u32)window_height };

  vulkan_create_frame_resources();

  SDL_srand(SDL_GetTicks());
  
  golfo_init();

  return SUCCESS;
}

internal void
sdl_process_input() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch(event.type) {
      case SDL_EVENT_QUIT:
        sdl_ctx.should_quit = true;
        break;

      case SDL_EVENT_WINDOW_RESIZED:
        SDL_WindowEvent *window_event = &event.window;
        vk_ctx.window_resized = true;
        vk_ctx.window_dim.width  = window_event->data1;
        vk_ctx.window_dim.height = window_event->data2;
        vk_ctx.resolution = vk_ctx.window_dim;
        break;

    }
  }
}

int main(int argc, char *argv[]) {
  if (sdl_init() == FAILURE) {
    return 1;
  }

  vulkan_clear_color({1, 1, 1, 1});

  while(1) {
    sdl_process_input();
    if (sdl_ctx.should_quit) {
      break;
    }

    if (vk_ctx.window_resized) {
      vulkan_destroy_frame_resources();
      vulkan_create_frame_resources();
      vk_ctx.window_resized = false;
    }

    golfo_update();
  }
  
  return 0;
}
