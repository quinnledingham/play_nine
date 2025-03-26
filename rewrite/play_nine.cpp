#include "play_nine.h"

/*
  0 = init was successfull
  1 = init failed
*/
s32 play_nine_init() {
  gfx_define_layouts();

  // asset loading
  s32 load_pipelines_result = load_pipelines();
  if (load_pipelines_result == FAILURE) {
    return FAILURE;
  }

  load_assets(&assets.fonts, font_loads, ARRAY_COUNT(font_loads), AT_FONT);
  load_assets(&assets.bitmaps, bitmap_loads, ARRAY_COUNT(bitmap_loads), AT_BITMAP);

  init_deck();

  test_game.players_count = 4;
  start_game(&test_game);
  
#ifdef DEBUG

  bool8 recreate_input_prompt_atlases = true;
  if (recreate_input_prompt_atlases) {
    Texture_Atlas atlases[PROMPT_COUNT];
    create_input_prompt_atlas(&atlases[PROMPT_KEYBOARD], keyboard_prompts, ARRAY_COUNT(keyboard_prompts), "../libs/xelu/Keyboard & Mouse/Dark/", "_Key_Dark.png", "prompt.png");
    //create_input_prompt_atlas(&atlases[PROMPT_XBOX_SERIES], xbox_prompts, ARRAY_COUNT(xbox_prompts), "../xelu/Xbox Series/XboxSeriesX_", ".png", "xbox_prompt.png");

    texture_atlas_write(&atlases[PROMPT_KEYBOARD], "keyboard.png", "keyboard.tco");
    //texture_atlas_write(&atlases[PROMPT_XBOX_SERIES], "xbox_series.png", "xbox_series.tco");
  }

#endif // DEBUG

  load_assets(&assets.atlases, atlas_loads, ARRAY_COUNT(atlas_loads), AT_ATLAS);

  return SUCCESS;
}

internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
    //state->scene.view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

    ortho_scene->view = identity_m4x4();
    ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}

s32 draw() {
  // Resets all of the descriptor sets to be reallocated on next frame
  for (u32 i = 0; i < GFXID_COUNT; i++) {
      gfx.layouts[i].reset();
  }

  vulkan_clear_color({0, 0, 0, 1});
  update_scenes(&scene, &ortho_scene, gfx.window.dim);

  if (gfx.window.resized) {
    vulkan_destroy_frame_resources();
    vulkan_create_frame_resources();
    gfx.window.resized = false;
  }

  if (vulkan_start_frame()) {
    return FAILURE;
  }

  start_draw_2D();

  //draw_rect({200, 200}, {300, 300}, {255, 0, 0, 1});

  Bitmap *bitmap = find_bitmap(BITMAP_LANA);
  //draw_rect({300, 200}, cv2(bitmap->dim/5), bitmap);

  set_draw_font(FONT_LIMELIGHT);
  //draw_text("LANA", {200, 100}, 100, {200, 100, 0, 1});

  draw_main_menu();

  //draw_rect({400 ,400}, {100, 100}, &find_atlas(ATLAS_KEYBOARD)->bitmap);
  //u32 index = find_input_prompt_index(last_key, keyboard_prompts, ARRAY_COUNT(keyboard_prompts));
  //texture_atlas_draw_rect(ATLAS_KEYBOARD, index, {400 ,400}, {100, 100});

  char fps[20];
  float_to_string(app_time.frames_per_s, fps, 20);
  set_draw_font(FONT_ROBOTO_MONO);
  draw_text(fps, {0, 0}, 20, {255, 0, 0, 1});
  
  end_draw_2D();
  
  vulkan_end_frame();

  return SUCCESS;
}

s32 update() {
  draw();

  return SUCCESS;
}
