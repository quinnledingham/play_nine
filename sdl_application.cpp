#include <SDL.h>

#ifdef WINDOWS

#pragma message("WINDOWS")
#define WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

//
// We prefer the discrete GPU in laptops where available
//
extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

#endif // WINDOWS

// Compiling to SPIR in code
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>

#include <spirv_cross/spirv_cross_c.h>

#ifdef OPENGL

#pragma message("OPENGL")
#include <gl.h>
#include <gl.c>

#elif VULKAN

#pragma message("VULKAN")
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
    
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

void *platform_malloc(u32 size) { return SDL_malloc(size); }
void platform_free(void *ptr)   { SDL_free(ptr); }
void platform_memory_copy(void *dest, void *src, u32 num_of_bytes) { SDL_memcpy(dest, src, num_of_bytes); }
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { SDL_memset(dest, value, num_of_bytes); }

#include "print.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "assets_loader.h"
#include "shapes.h"
#include "data_structs.h"
#include "render.h"
#include "shaders.h"
#include "application.h"

#include "print.cpp"
#include "assets.cpp"
#include "obj.cpp"
#include "assets_loader.cpp"
#include "shapes.cpp"
#include "render.cpp"

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
    time->frame_time_s = (float32)get_seconds_elapsed(time, time->last_frame_ticks, ticks);
    time->last_frame_ticks = ticks; // set last ticks for next frame

    // time->start has to be initialized before
    time->run_time_s = (float32)get_seconds_elapsed(time, time->start_ticks, ticks);

    // fps
    time->frames_per_s = (float32)(1.0 / time->frame_time_s);
}
internal bool8
sdl_process_input(App *app, App_Window *window, App_Input *input) {
    window->resized = false;

    input->mouse = {};
    input->mouse_rel = {};
    input->key_events_count = 0;
    
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT: return true;

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
                    } break;
                }
            } break;

            case SDL_MOUSEMOTION: {
                SDL_MouseMotionEvent *mouse_motion_event = &event.motion;
                input->mouse.x = mouse_motion_event->x;
                input->mouse.y = mouse_motion_event->y;
                input->mouse_rel.x = mouse_motion_event->xrel;
                input->mouse_rel.y = mouse_motion_event->yrel;
            } break;

            case SDL_KEYDOWN: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                u32 key_id = keyboard_event->keysym.sym;
                event_handler(app, APP_KEYDOWN, key_id);
            } break;

            case SDL_KEYUP: {
                SDL_KeyboardEvent *keyboard_event = &event.key;
                u32 key_id = keyboard_event->keysym.sym;
                event_handler(app, APP_KEYUP, key_id);
            } break;
		}
	}

	return false;
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

    SDL_GetWindowSize(sdl_window, &app.window.width, &app.window.height);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    render_sdl_init(sdl_window);
    render_clear_color(Vector4{ 0.0f, 0.2f, 0.4f, 1.0f });

    event_handler(&app, APP_INIT, 0);    

    init_shapes();

    srand(SDL_GetTicks());

    while (1) {
        if (app.input.relative_mouse_mode) SDL_SetRelativeMouseMode(SDL_TRUE);
        else                               SDL_SetRelativeMouseMode(SDL_FALSE);

    	if (sdl_process_input(&app, &app.window, &app.input)) 
            return 0;
        
    	sdl_update_time(&app.time);
        //print("%f\n", app.time.frames_per_s);

        app.update(&app);
    }
    
    render_cleanup();
    SDL_DestroyWindow(sdl_window);

	return 0;
}
