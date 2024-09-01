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

void draw_string(Font *font, const char *string, Vector2 coords, float32 pixel_height, Vector4 color) {
    float32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    if (scale == 0.0f)
        return;

    float32 string_x_coord = 0.0f;

    float32 current_point = coords.x;
    float32 baseline      = coords.y;

    Texture_Atlas *atlas = &font->cache->atlas;
    
    gfx_bind_shader("TEXT");
    render_bind_descriptor_sets(shapes_color_descriptor, &color);
    render_bind_descriptor_set(atlas->descs[gfx.current_frame]);

    Object object = {};
    Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });

    Font_Char *font_char = 0;
    Font_Char *font_char_next = 0;
    u32 string_index = 0;
    while(string[string_index] != 0) {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[string_index], scale);
        
        font_char = bitmap->font_char;
        font_char_next = load_font_char(font, string[string_index + 1]);

        // Draw
        if (bitmap->bitmap.width != 0) {
            Vector2 char_coords = { current_point + (font_char->lsb * scale), baseline + (float32)bitmap->bb_0.y };

            VkDeviceSize offsets[] = { vulkan_info.dynamic_buffer.offset };
            Texture_Coords tex_coord = atlas->texture_coords[bitmap->index];
            Vector2 coords = { char_coords.x, char_coords.y };
            Vector2 dim = { (float32)bitmap->bitmap.width, (float32)bitmap->bitmap.height };

            //coords.x += dim.x / 2.0f;
            //coords.y += dim.y / 2.0f; // coords = top left corner
            
            vulkan_immediate_vertex(Vertex_XU{ coords, tex_coord.p1 });
            vulkan_immediate_vertex(Vertex_XU{ coords + dim, tex_coord.p2 });
            vulkan_immediate_vertex(Vertex_XU{ { coords.x, coords.y + dim.y }, {tex_coord.p1.x, tex_coord.p2.y} });

            vulkan_immediate_vertex(Vertex_XU{ coords, tex_coord.p1 });
            vulkan_immediate_vertex(Vertex_XU{ { coords.x + dim.x, coords.y }, {tex_coord.p2.x, tex_coord.p1.y} });
            vulkan_immediate_vertex(Vertex_XU{ coords + dim, tex_coord.p2 });

            Object object = {};
            object.model = identity_m4x4();
            render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

            vkCmdBindVertexBuffers(VK_CMD(vulkan_info), 0, 1, &vulkan_info.dynamic_buffer.handle, offsets);
            vkCmdDraw(VK_CMD(vulkan_info), 6, 1, 0, 0);
        }
        // End of Draw
        s32 kern = get_glyph_kern_advance(font->info, font_char->glyph_index, font_char_next->glyph_index);
        current_point += scale * (kern + font_char->ax);

        string_index++;
    }
    
}
