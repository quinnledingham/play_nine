#ifndef CLAY2_H
#define CLAY2_H

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

#endif // CLAY2_H