#ifndef CLAY_IRU_H
#define CLAY_IRU_H

extern "C" void init_clay(SDL_Renderer *renderer, float width, float height);
extern "C" void draw_clay();
extern "C" void clay_set_layout(float width, float height);
extern "C" void clay_set_pointer_state(float x, float y, bool left);
extern "C" void clay_update_scroll_containers(float x, float y);
extern "C" void clay_set_renderer(SDL_Renderer *renderer);

#endif // CLAY_IRU_H