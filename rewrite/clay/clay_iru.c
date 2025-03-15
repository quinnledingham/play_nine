#define CLAY_IMPLEMENTATION
#include "../clay/clay.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_image/SDL_image.h>

#include "../defines.h"

typedef struct {
    SDL_Renderer *renderer;
    TTF_TextEngine *textEngine;
    TTF_Font **fonts;
} Clay_SDL3RendererData;

typedef struct {
    intptr_t offset;
    intptr_t memory;
} ClayVideoDemo_Arena;

typedef struct {
    int32_t selectedDocumentIndex;
    float yOffset;
    ClayVideoDemo_Arena frameArena;
} ClayVideoDemo_Data;

Clay_SDL3RendererData renderer_data;
ClayVideoDemo_Data demo_data;

#include "../clay/clay-video-demo.c"
#include "../clay/clay_renderer_SDL3.c"

void clay_set_renderer(SDL_Renderer *renderer) {
  renderer_data.renderer = renderer;
  renderer_data.textEngine = TTF_CreateRendererTextEngine(renderer);
  if (!renderer_data.textEngine) {
    SDL_Log("Failed to create text engine from renderer: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
}

void handle_clay_errors(Clay_ErrorData error_data) {
    SDL_Log("%s", error_data.errorText.chars);
}

void init_clay(SDL_Renderer *renderer, float width, float height) {
  clay_set_renderer(renderer);

  renderer_data.fonts = (TTF_Font **)malloc(sizeof(TTF_Font*) * 1);
  TTF_Font *font = TTF_OpenFont("../assets/fonts/Roboto-Regular.ttf", 24);
  if (!font) {
    SDL_Log("Failed to load font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }
  renderer_data.fonts[0] = font;

  /* Initialize Clay */
  u64 total_memory_size = Clay_MinMemorySize();
  Clay_Arena clay_memory = Clay_CreateArenaWithCapacityAndMemory(total_memory_size, (char *)malloc(total_memory_size));

  Clay_Initialize(clay_memory, (Clay_Dimensions) { (float) width, (float) height }, (Clay_ErrorHandler) { handle_clay_errors });
  Clay_SetMeasureTextFunction(SDL_MeasureText, renderer_data.fonts);

  demo_data = ClayVideoDemo_Initialize();
}

void draw_clay() {
  Clay_RenderCommandArray render_commands = ClayVideoDemo_CreateLayout(&demo_data);

  SDL_SetRenderDrawColor(renderer_data.renderer, 0, 0, 0, 255);
  SDL_RenderClear(renderer_data.renderer);

  SDL_Clay_RenderClayCommands2(&renderer_data, &render_commands);

  SDL_RenderPresent(renderer_data.renderer);
}

void clay_set_layout(float width, float height) {
  Clay_SetLayoutDimensions( (Clay_Dimensions) { width, height });
}

void clay_set_pointer_state(float x, float y, bool left) {
  Clay_SetPointerState( (Clay_Vector2) { x, y }, left);
}

void clay_update_scroll_containers(float x, float y) {
  Clay_UpdateScrollContainers(true, (Clay_Vector2) { x, y }, 0.05f);
}
