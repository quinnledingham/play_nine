//
// Loads in the basic library code
//
// Sets up operating system libs and gpu libs aswell as quirk code
//

#ifdef OS_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")

//
// We prefer the discrete GPU in laptops where available
//
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

#elif OS_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>

#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>

#include <algorithm>
using namespace std;

#endif // OS_WINDOWS / OS_LINUX

// Compiling to SPIR in code
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>

#include <spirv_cross/spirv_cross_c.h>

#ifdef OPENGL

#include <gl.h>

#elif VULKAN

#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
    
// Provided by VK_EXT_shader_object
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderEXT)

#define VK_USE_PLATFORM_WIN32_KHR

#elif DX12

#include <wingdi.h>
#include <windef.h>
#include <winuser.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include "dx12/d3dx12.h" // Helper Structures and Functions

#include <string>
#include <wrl.h>
using namespace Microsoft::WRL;
#include <shellapi.h>

#include <mmsystem.h>
#include <dsound.h>
#include <intrin.h>
#include <xinput.h>

#endif // OPENGL / VULKAN / DX12

#include <stdarg.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// quirk
#include "defines.h"
#include "types.h"

void* platform_malloc(u32 size);
void platform_free(void *ptr);
void platform_memory_copy(void *dest, const void *src, u32 num_of_bytes);
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes);

#ifdef SDL

#include <SDL.h>
#include <SDL_gamecontroller.h>

void *platform_malloc(u32 size) { 
    return SDL_malloc(size); 
}
void platform_free(void *ptr)   { 
    SDL_free(ptr); 
}
void platform_memory_copy(void *dest, const void *src, u32 num_of_bytes) { 
    SDL_memcpy(dest, src, num_of_bytes); 
}
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { 
    SDL_memset(dest, value, num_of_bytes); 
}

#endif // SDL

#include "print.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "assets_loader.h"
#include "data_structs.h"

#include "thread.h"
#ifdef OS_WINDOWS
#include "win32_thread.h"
#elif OS_LINUX
#include "linux_thread.h"
#endif // OS

#include "shapes.h"
#include "application.h"
#include "render.h"
#include "input.h"
#include "gui.h"
#include "qsock.h"

#include "print.cpp"
#include "assets.cpp"
#include "obj.cpp"
#include "assets_loader.cpp"
#include "shapes.cpp"
#include "render.cpp"
#include "gui.cpp"
//#include "input.cpp"

#ifdef OPENGL

#include "opengl.h"
#include "opengl.cpp"

#elif VULKAN

#include "vulkan.h"
#include "vulkan.cpp"

#elif DX12

#include "dx12.h"
#include "dx12.cpp"

#endif // OPENGL / VULKAN / DX12

#ifdef STEAM

#include "steam_api.h"
#include "isteamfriends.h"
#include "isteammatchmaking.h"

#endif // STEAM

#include "sdl_application.cpp"
