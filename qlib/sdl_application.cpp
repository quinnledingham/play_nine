#include <SDL.h>

//#define OS_WINDOWS
//#define API_SDL
//#define API_VULKAN

#ifdef OS_WINDOWS

#pragma message("WINDOWS")

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

#endif // WINDOWS

// Compiling to SPIR in code
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>

#include <spirv_cross/spirv_cross_c.h>

#ifdef OPENGL

//#pragma message("OPENGL")

#include <gl.h>
#include <gl.c>

#elif VULKAN

//#pragma message("VULKAN")
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

#include "defines.h"
#include "types.h"

void *platform_malloc(u32 size) { 
    return SDL_malloc(size); 
}
void platform_free(void *ptr)   { 
    SDL_free(ptr); 
}
void platform_memory_copy(void *dest, void *src, u32 num_of_bytes) { 
    SDL_memcpy(dest, src, num_of_bytes); 
}
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { 
    SDL_memset(dest, value, num_of_bytes); 
}

void * array_malloc(u32 size, u32 n) {
    return platform_malloc(n * size);
}

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

#include "render.h"
#include "shapes.h"
#include "play_nine_raytrace.h"
#include "play_nine_shaders.h"
#include "application.h"
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

#include "play_nine.cpp"

internal void
sdl_update_time(App_Time *time) {
    s64 ticks = SDL_GetPerformanceCounter();

    // s
    time->frame_time_s = get_seconds_elapsed(time, time->last_frame_ticks, ticks);
    time->last_frame_ticks = ticks; // set last ticks for next frame

    // time->start has to be initialized before
    time->run_time_s = get_seconds_elapsed(time, time->start_ticks, ticks);

    // fps
    time->frames_per_s = 1.0 / time->frame_time_s;
}

internal void
sdl_set_fullscreen(SDL_Window *sdl_window, enum Display_Modes display_mode) {
    switch(display_mode) {
        case DISPLAY_MODE_WINDOWED: SDL_SetWindowFullscreen(sdl_window, 0); break;
        case DISPLAY_MODE_FULLSCREEN: SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN); break;
        case DISPLAY_MODE_WINDOWED_FULLSCREEN: SDL_SetWindowFullscreen(sdl_window, SDL_WINDOW_FULLSCREEN_DESKTOP); break;
    }
}

internal bool8
sdl_process_input(App *app, App_Window *window, App_Input *input, SDL_Window *sdl_window) {
    window->resized = false;

    //input->mouse = {};
    input->mouse_rel = {};
    input->buffer_index = 0;
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT: {
                return true;
            } break;

			case SDL_WINDOWEVENT: {
                SDL_WindowEvent *window_event = &event.window;
                
                switch(window_event->event) {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        window->width  = window_event->data1;
                        window->height = window_event->data2;
                        window->resized = true;
                        
						        #ifdef OPENGL
                        //opengl_update_window(window);
                    #elif VULKAN
                        vulkan_info.window_width = window->width;
                        vulkan_info.window_height = window->height;
                        vulkan_info.framebuffer_resized = true;
						        #elif DX12
						            dx12_resize_window(&dx12_renderer, window);
						        #endif // OPENGL / DX12
                        event_handler(app, APP_RESIZED, 0);
                    } break;

                    case SDL_WINDOWEVENT_MINIMIZED: {
                        app->window.minimized = true;
                    } break;

                    case SDL_WINDOWEVENT_RESTORED: {
                        app->window.minimized = false;
                    } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                SDL_MouseMotionEvent *mouse_motion_event = &event.motion;
                input->mouse.x = mouse_motion_event->x;
                input->mouse.y = mouse_motion_event->y;
                input->mouse_rel.x += mouse_motion_event->xrel;
                input->mouse_rel.y += mouse_motion_event->yrel;

                input->active = MOUSE_INPUT;
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                SDL_MouseButtonEvent *mouse_event = &event.button;
                event_handler(app, APP_MOUSEDOWN, mouse_event->button);
            } break;

            case SDL_MOUSEBUTTONUP: {
                SDL_MouseButtonEvent *mouse_event = &event.button;
                input->mouse = { mouse_event->x, mouse_event->y };
                event_handler(app, APP_MOUSEUP, mouse_event->button);
            } break;

            case SDL_KEYDOWN: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                bool32 shift = keyboard_event->keysym.mod & KMOD_LSHIFT;
                u32 key_id = keyboard_event->keysym.sym;
                if (!app_input_buffer)
                    event_handler(app, APP_KEYDOWN, key_id);
                else {
                    if (shift) {
                        key_id -= 32;
                    }
                    s32 ch = 0;
                    if (is_ascii(key_id))
                        ch = key_id;
                    else if (key_id == SDLK_LEFT)  ch = 37;
                    else if (key_id == SDLK_UP)    ch = 38;
                    else if (key_id == SDLK_RIGHT) ch = 39;
                    else if (key_id == SDLK_DOWN)  ch = 40;
                    
                    if (ch != 0)
                        input->buffer[input->buffer_index++] = ch;
                }
            } break;

            case SDL_KEYUP: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                u32 key_id = keyboard_event->keysym.sym;
                if (key_id == SDLK_F11) {
                    app_toggle_fullscreen(window);
                   break;
                }
                event_handler(app, APP_KEYUP, key_id);
            } break;    
		}
	}

    app_input_buffer = false;
	return false;
}

internal void
sdl_set_icon(Bitmap *icon, SDL_Window *sdl_window) {
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceFrom(icon->memory, icon->dim.width, icon->dim.height, 32, icon->pitch, 0x00000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_SetWindowIcon(sdl_window, icon_surface);
    SDL_FreeSurface(icon_surface);
}

int main(int argc, char *argv[]) {
	print("starting application...\n");

	App app = {};

	app.time.performance_frequency = SDL_GetPerformanceFrequency();
    app.time.start_ticks           = SDL_GetPerformanceCounter();
    app.time.last_frame_ticks      = app.time.start_ticks;
    
	u32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO;
    if (SDL_Init(sdl_init_flags)) {
    	print(SDL_GetError());
    	return 1;
    }

    qsock_init_qsock();

    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE;
#ifdef OPENGL
    sdl_window_flags |= SDL_WINDOW_OPENGL;
#elif VULKAN
    sdl_window_flags |= SDL_WINDOW_VULKAN;
#elif DX12
    
#endif // OPENGL / DX12

    SDL_Window *sdl_window = SDL_CreateWindow("play_nine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 800, sdl_window_flags);
    if (sdl_window == NULL) {
    	print(SDL_GetError());
    	return 1;
    }

    if (SDL_HasScreenKeyboardSupport() == SDL_TRUE)
        print("Has Screen Keyboard Support!\n");
    
    SDL_GetWindowSize(sdl_window, &app.window.width, &app.window.height);
    SDL_SetRelativeMouseMode(SDL_FALSE);

    if (render_sdl_init(sdl_window)) {
        logprint("main()", "Failed to init renderer\n");
        return 1;
    }
    
    if (event_handler(&app, APP_INIT, 0) == 1)
        return 1;

    sdl_set_icon(app.icon, sdl_window);
    
    srand(SDL_GetTicks());

    while (1) {
        if (app.input.relative_mouse_mode) 
            SDL_SetRelativeMouseMode(SDL_TRUE);
        else 
            SDL_SetRelativeMouseMode(SDL_FALSE);

        if (sdl_process_input(&app, &app.window, &app.input, sdl_window)) 
            break;
        sdl_set_fullscreen(sdl_window, app.window.display_mode);

        sdl_update_time(&app.time);
        //print("%f\n", app.time.frames_per_s);
        //print("%f\n", app.time.run_time_s);

        window_dim = app.window.dim;

        if (app.window.minimized)
            continue;

        if (app.update(&app))
            break;

        //SDL_Delay(5);
    }

    render_wait_frame();
    event_handler(&app, APP_EXIT, 0) ;
    render_cleanup();
    SDL_DestroyWindow(sdl_window);

	return 0;
}
