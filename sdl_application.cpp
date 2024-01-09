#ifdef WINDOWS

	#define WIN32_EXTRA_LEAN
	#define WIN32_EXTRA_LEAN
	#include <windows.h>

#endif // WINDOWS

#ifdef OPENGL

	#include <gl.h>
	#include <gl.c>

#endif // OPENGL

#ifdef DX12

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

#endif // DX12

#include <stdarg.h>
#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define internal      static
#define local_persist static
#define global        static

#include <SDL.h>

#include "types.h"

void *platform_malloc(u32 size) { return SDL_malloc(size); }
void platform_free(void *ptr)   { SDL_free(ptr); }
void platform_memory_copy(void *dest, void *src, u32 num_of_bytes) { SDL_memcpy(dest, src, num_of_bytes); }
void platform_memory_set(void *dest, s32 value, u32 num_of_bytes) { SDL_memset(dest, value, num_of_bytes); }

#define ARRAY_COUNT(n)     (sizeof(n) / sizeof(n[0]))
#define ARRAY_MALLOC(t, n) ((t*)platform_malloc(n * sizeof(t)))

//
// NOTE(casey): We prefer the discrete GPU in laptops where available
//
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

#include "print.h"
#include "char_array.h"

#include "assets.h"
#include "shapes.h"
#include "types_math.h"
#include "application.h"
#include "renderer.h"

bool8 update(Application *app);

#include "print.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "assets.cpp"
#include "shapes.cpp"

internal void
sdl_update_time(Application_Time *time) {
    s64 ticks = SDL_GetPerformanceCounter();

    // s
    time->frame_time_s = (float32)get_seconds_elapsed(time, time->last_frame_ticks, ticks);
    time->last_frame_ticks = ticks; // set last ticks for next frame

    // time->start has to be initialized before
    time->run_time_s = (float32)get_seconds_elapsed(time, time->start_ticks, ticks);

    // fps
    time->frames_per_s = (float32)(1.0 / time->frame_time_s);
}

#ifdef OPENGL

internal void
opengl_update_window(Application_Window *window) {
	glViewport(0, 0, window->width, window->height);
    window->aspect_ratio = (float32)window->width / (float32)window->height;
}

internal void
sdl_init_opengl(SDL_Window *sdl_window) {
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,    1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_SetSwapInterval(0); // vsync: 0 off, 1 on
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    print("OpenGL loaded:\n");
    print("Vendor:   %s\n", glGetString(GL_VENDOR));
    print("Renderer: %s\n", glGetString(GL_RENDERER));
    print("Version:  %s\n", glGetString(GL_VERSION));

    //
    // DEFAULTS
    //


}

#endif // OPENGL

#ifdef DX12

	#include "dx12.h"
	#include "dx12.cpp"

#endif // DX12

internal bool8
sdl_process_input(Application_Window *window, Application_Input *input) {
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
						    opengl_update_window(window);
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

	Application app = {};

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
#elif DX12
    u32 sdl_window_flags = SDL_WINDOW_RESIZABLE;
#endif // OPENGL / DX12

    SDL_Window *sdl_window = SDL_CreateWindow("play_nine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 900, 800, sdl_window_flags);
    if (sdl_window == NULL) {
    	print(SDL_GetError());
    	return 1;
    }

    SDL_GetWindowSize(sdl_window, &app.window.width, &app.window.height);

#ifdef OPENGL
    sdl_init_opengl(sdl_window);
    opengl_update_window(&app.window);
#elif DX12
    dx12_init(&dx12_renderer, &app.window);
#endif // OPENGL / DX12

    //srand(SDL_GetTicks()); // setup randomizer

    //init_shapes();
    Triangle_Mesh test = get_rect_triangle_mesh();

    renderer_clear_color = Vector4{ 0.0f, 0.2f, 0.4f, 1.0f };

    while (1) {

    	if (sdl_process_input(&app.window, &app.input)) return 0;

    	sdl_update_time(&app.time);

#ifdef DX12
        dx12_start_commands(&dx12_renderer);
#endif // DX12

        draw_triangle_mesh(&test);
    	if(update(&app)) return 0;

#ifdef OPENGL
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glClearColor(renderer_clear_color.r, renderer_clear_color.g, renderer_clear_color.b, renderer_clear_color.a);
        glPatchParameteri(GL_PATCH_VERTICES, 4); 

    	SDL_GL_SwapWindow(sdl_window);

    	u32 gl_clear_flags = 
            GL_COLOR_BUFFER_BIT  | 
            GL_DEPTH_BUFFER_BIT  | 
            GL_STENCIL_BUFFER_BIT;
    
        glClear(gl_clear_flags);
#elif DX12
        dx12_render(&dx12_renderer);
#endif // OPENGL DX12
    }

	return 0;
}