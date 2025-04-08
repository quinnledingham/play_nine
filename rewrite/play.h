#ifdef OS_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

//#pragma comment(lib, "Ws2_32.lib")

//
// We prefer the discrete GPU in laptops where available
//
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

#endif // OS

#include <SDL.h>
#include <SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include <stdio.h>
#include <vector>

// Compiling to SPIR in code
//#ifdef DEBUG
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>
//#endif // DEBUG

#include <spirv_cross/spirv_cross_c.h>

#ifdef GFX_VULKAN

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// Provided by VK_EXT_shader_object
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderEXT)

#define VK_USE_PLATFORM_WIN32_KHR

#endif // GFX_VULKAN

//#include "clay/clay_iru.h"

#define WINDOW_NAME "Deck Nine"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN
#define MAX_FRAMES_IN_FLIGHT 2

#include "defines.h"
#include "types.h"
#include "log.h"
#include "types_math.h"
#include "data_structs.h"
#include "str.h"
#include "gfx_layouts.h"
struct Vulkan_Buffer;
#include "assets.h"
#include "vulkan.h"
#include "sdl.h"
#include "gfx.h"
#include "gui.h"

#include "play_game.h"
#include "play_draw.h"
#include "play_raytrace.h"

#include "input.h"
#include "global.h"

#include "play_assets.h"

s32 play_init();
s32 do_game_frame();
void play_destroy();

#include "log.cpp"
#include "vulkan.cpp"
#include "geometry.cpp"
#include "assets.cpp"
#include "gfx.cpp"
#include "draw.cpp"
#include "input_prompts.cpp"
#include "sdl.cpp"
#include "gui.cpp"

#include "play_raytrace.cpp"
#include "play_animations.cpp"
#include "play_game.cpp"
#include "play_bitmaps.cpp"
#include "play_draw.cpp"
#include "play_menus.cpp"

/*
 TODO:

 GUI
 - Speed up rounded rect

 ASSETS
 - Load obj and mtl files
 
 GAME DRAWING
 - dont be able to point below card so that it lifts above hover then drops
back down to it repeated.

*/