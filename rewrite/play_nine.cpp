#include "play_nine.h"

/*
  0 = init was successfull
  1 = init failed
*/
s32 play_nine_init() {
  // asset loading
  s32 load_pipelines_result = load_pipelines();
  if (load_pipelines_result == FAILURE) {
    return FAILURE;
  }

  load_fonts();

  init_deck();

  test_game.players_count = 4;
  start_game(&test_game);
  
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

  vulkan_clear_color(gfx.vk_ctx, {1, 0, 1, 1});
  update_scenes(&scene, &ortho_scene, gfx.window.dim);

  if (gfx.window.resized) {
    vulkan_destroy_frame_resources(gfx.vk_ctx);
    vulkan_create_frame_resources(gfx.vk_ctx, &gfx);
    gfx.window.resized = false;
  }

  if (vulkan_start_frame(gfx.vk_ctx)) {
    return FAILURE;
  }

  gfx.default_viewport();
  gfx.default_scissor();
  vulkan_depth_test(gfx.vk_ctx, false);

  draw_rect({200, 200}, {300, 300}, {255, 0, 0, 1});

  vulkan_end_frame(gfx.vk_ctx, gfx.resolution_scaling, gfx.window.resized);

  return SUCCESS;
}

s32 update() {

/*
  draw_game(&test_game);

  char test[50];
  scanf("%s", test);
  u32 input_value = atoi(test);

  if (!strcmp(test, "quit")) {
    print("QUIT\n");
    return FAILURE;
  }

  bool8 input[GI_SIZE] = {};
  if (input_value < GI_SIZE) {
    input[input_value] = true;
  }
  update_game_with_input(&test_game, input);

  s32 draw_result = draw();
  if (draw_result == FAILURE) {
    log_error("update(): draw failed\n");
  }
*/

  draw();
  //draw_game_2D(&test_game);

  return SUCCESS;
}
