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

#ifdef API3D_VULKAN

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// Provided by VK_EXT_shader_object
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderEXT)

#define VK_USE_PLATFORM_WIN32_KHR

#endif // API3D_VULKAN

#define WINDOW_NAME "pinball"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 500
#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN

#include "defines.h"
#include "types.h"
#include "log.h"
#include "types_math.h"
#include "data_structs.h"
#include "str.h"
#include "gfx_layouts.h"
#include "assets.h"
#include "vulkan.h"
#include "gfx.h"

#include "global.h"

#include "play_nine_assets.h"

s32 init();
s32 update();

#include "log.cpp"
#include "assets.cpp"
#include "vulkan.cpp"
#include "gfx.cpp"
#include "sdl.cpp"
#include "shapes.cpp"