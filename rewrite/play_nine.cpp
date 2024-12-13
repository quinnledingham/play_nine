#include "basic.h"

#include "play_nine_assets.h"

bool8 update() {
  /*
  gfx_attach_bitmap(BITMAP_BOT);
  play_sound(SOUND_BOOP);
  */
  //printf("FPS: %f\n", app.time.frames_per_s);

  return 0;
}

void init() {
  app.update = update;

  // initialize shader asset array
  // kinda like setting the enums for each type of shader
  allocate_assets(ASSET_TYPE_SHADER, SHADER_COUNT);
  allocate_assets(ASSET_TYPE_BITMAP, BITMAP_COUNT);

  add_asset(ASSET_TYPE_SHADER, SHADER_COLOR, "2D.vert");
  add_asset(ASSET_TYPE_SHADER, SHADER_COLOR, "color.frag");

  load_assets(&app.assets);

  Shader *test = (Shader *)app.assets.arrays[ASSET_TYPE_SHADER].memory;
}

void quit() {
  printf("QUITING\n");
}

void event_handler(u32 event) {
  switch (event) {
    case EVENT_INIT: init(); break;
    case EVENT_QUIT: quit(); break;
  }
}