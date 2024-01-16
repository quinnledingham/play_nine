#include <SDL.h>

#ifdef WINDOWS

#pragma message("WINDOWS")
#define WIN32_EXTRA_LEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

//
// We prefer the discrete GPU in laptops where available
//
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

#endif // WINDOWS

#ifdef OPENGL

#pragma message("OPENGL")
#include <gl.h>
#include <gl.c>

#elif VULKAN

#pragma message("VULKAN")
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>
    
// Compiling to SPIR in code
#include <shaderc/env.h>
#include <shaderc/shaderc.h>
#include <shaderc/shaderc.hpp>
#include <shaderc/status.h>
#include <shaderc/visibility.h>

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

#include "types.h"

void *platform_malloc(u32 size) { return SDL_malloc(size); }
void platform_free(void *ptr)   { SDL_free(ptr); }
void platform_memory_copy(void *dest, void *src, u32 num_of_bytes) { SDL_memcpy(dest, src, num_of_bytes); }
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { SDL_memset(dest, value, num_of_bytes); }

#define ARRAY_COUNT(n)     (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n) ((t*)platform_malloc(n * sizeof(t)))

#include "print.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "shapes.h"
#include "data_structs.h"
#include "render.h"
#include "application.h"

bool8 update(App *app);

#include "print.cpp"
#include "assets.cpp"
#include "shapes.cpp"
#include "play_nine.cpp"

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
sdl_process_input(App_Window *window, App_Input *input) {
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT: return true;

			case SDL_WINDOWEVENT:
            {
                SDL_WindowEvent *window_event = &event.window;
                
                switch(window_event->event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window->width  = window_event->data1;
                        window->height = window_event->data2;
                        
						#ifdef OPENGL
						    //opengl_update_window(window);
						#elif DX12
						    dx12_resize_window(&dx12_renderer, window);
						#endif // OPENGL / DX12
                    } break;
                }
            } break;
		}
	}

	return false;
}

int main(int argc, char *argv[]) {
	print("starting application...\n");

	App app = {};

	app.time.performance_frequency = SDL_GetPerformanceFrequency();
    app.time.start_ticks = SDL_GetPerformanceCounter();
    app.time.last_frame_ticks = app.time.start_ticks;

	u32 sdl_init_flags = SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO;
    if (SDL_Init(sdl_init_flags)) {
    	print(SDL_GetError());
    	return 1;
    }

#ifdef OPENGL
    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL;
#elif VULKAN
    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
#elif DX12
    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE;
#endif // OPENGL / DX12

    SDL_Window *sdl_window = SDL_CreateWindow("play_nine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 800, sdl_window_flags);
    if (sdl_window == NULL) {
    	print(SDL_GetError());
    	return 1;
    }

    SDL_GetWindowSize(sdl_window, &app.window.width, &app.window.height);

    render_sdl_init(sdl_window);

    render_clear_color(Vector4{ 0.0f, 0.2f, 0.4f, 1.0f });
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_BLEND);

    Descriptor_Set set = {};
    set.descriptors[0] = Descriptor(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX);
    set.descriptors[1] = Descriptor(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT);
    set.descriptors_count = 2;
    vulkan_create_descriptor_sets(&set);

    vulkan_create_graphics_pipeline(&vulkan_info, &set);
    
    Bitmap yogi = load_bitmap("../assets/bitmaps/yogi.png");
    vulkan_create_texture(&yogi);
    free_bitmap(yogi);

    // tell where image is
    vulkan_update_descriptor_sets(&set, &yogi);

    Scene scene = {};
    scene.model = create_transform_m4x4({ 0.0f, 0.0f, 0.0f }, get_rotation(0.0f, {0, 0, 1}), {1.0f, 1.0f, 1.0f});
    scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });
    scene.projection = perspective_projection(45.0f, (float32)app.window.width / (float32)app.window.height, 0.1f, 10.0f);
    
    // Init ubo
    Uniform_Buffer_Object scene_ubo = {};
    scene_ubo.offsets[0] = 0;
    scene_ubo.offsets[1] = (u32)vulkan_get_alignment(scene_ubo.offsets[0] + sizeof(Scene), (u32)vulkan_info.uniform_buffer_min_alignment);
    scene_ubo.size = sizeof(Scene);
    
    // this is the defining of where gpu memory to a uniform
    vulkan_update_descriptor_set(&set, &scene_ubo);

    // updates the gpu memory
    render_update_uniform_buffer_object(&scene_ubo, (void*)&scene, sizeof(Scene));

    Mesh rect = get_rect_mesh();

    while (1) {

    	if (sdl_process_input(&app.window, &app.input)) 
            return 0;

    	sdl_update_time(&app.time);

        render_start_frame();
        render_set_viewport(app.window.width, app.window.height);
        render_set_scissor(app.window.width, app.window.height);
        render_bind_pipeline();
        render_bind_descriptor_sets(&set);

        render_draw_mesh(&rect);

        render_end_frame();
    }
    
    render_cleanup();
    SDL_DestroyWindow(sdl_window);

	return 0;
}