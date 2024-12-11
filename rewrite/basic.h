#include <SDL.h>
#include <stdio.h>
#include <vector>

// Compiling to SPIR in code
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>

#include <spirv_cross/spirv_cross_c.h>

#ifdef API3D_VULKAN

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>

// Provided by VK_EXT_shader_object
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderEXT)

#define VK_USE_PLATFORM_WIN32_KHR

#endif // API3D_VULKAN

#include "defines.h"
#include "types.h"
#include "assets.h"
#include "app.h"

void event_handler(u32 event);

#define WINDOW_NAME "play_nine"
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define WINDOW_FLAGS SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN

#include "vulkan.h"
#include "gfx.h"

#include "assets.cpp"
#include "vulkan.cpp"
#include "app.cpp"
#include "sdl.cpp"