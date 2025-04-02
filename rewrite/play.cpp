#include "play.h"

internal void
set_button_ids() {
  button_set(&app_input.back, SDLK_ESCAPE);

  button_set(&app_input.forward, SDLK_W);
  button_set(&app_input.forward, SDLK_UP);
  button_set(&app_input.backward, SDLK_S);
  button_set(&app_input.backward, SDLK_DOWN);
  button_set(&app_input.left, SDLK_A);
  button_set(&app_input.left, SDLK_LEFT);
  button_set(&app_input.right, SDLK_D);
  button_set(&app_input.right, SDLK_RIGHT);
  button_set(&app_input.up, SDLK_SPACE);
  button_set(&app_input.down, SDLK_LSHIFT);
  button_set(&app_input.refresh_shaders, SDLK_R);
}

/*
  0 = init was successfull
  1 = init failed
*/
s32 play_init() {

  //
  // asset loading
  //
  gfx_define_layouts();
  prepare_asset_array(&assets.mtllibs, 1, sizeof(Material_Library)); // create blank mtllib array to add on to

  Load_Assets_Args args[5] = {
    { &assets.pipelines, pipeline_loads, ARRAY_COUNT(pipeline_loads), AT_SHADER },
    { &assets.fonts, font_loads, ARRAY_COUNT(font_loads), AT_FONT },
    { &assets.bitmaps, bitmap_loads, ARRAY_COUNT(bitmap_loads), AT_BITMAP },
    { &assets.geometrys, geometry_loads, ARRAY_COUNT(geometry_loads), AT_GEOMETRY },
    { &assets.atlases, atlas_loads, ARRAY_COUNT(atlas_loads), AT_ATLAS },
  };

  const u32 args_count = ARRAY_COUNT(args);
  SDL_Thread *asset_threads[args_count];
  for (u32 i = 0; i < ARRAY_COUNT(args); i++) {
    asset_threads[i] = SDL_CreateThread(load_assets_thread, "AssetThread", &args[i]);
  }
  
  for (u32 i = 0; i < args_count; i++) {
    sdl_wait_thread(asset_threads[i]);
  }

  init_assets();

#ifdef DEBUG

  if (debug.recreate_input_prompt_atlases) {
    Texture_Atlas atlases[PROMPT_COUNT];
    create_input_prompt_atlas(&atlases[PROMPT_KEYBOARD], keyboard_prompts, ARRAY_COUNT(keyboard_prompts), "../libs/xelu/Keyboard & Mouse/Dark/", "_Key_Dark.png", "prompt.png");
    //create_input_prompt_atlas(&atlases[PROMPT_XBOX_SERIES], xbox_prompts, ARRAY_COUNT(xbox_prompts), "../xelu/Xbox Series/XboxSeriesX_", ".png", "xbox_prompt.png");

    texture_atlas_write(&atlases[PROMPT_KEYBOARD], "keyboard.png", "keyboard.tco");
    //texture_atlas_write(&atlases[PROMPT_XBOX_SERIES], "xbox_series.png", "xbox_series.tco");
  }

  if (debug.recreate_bitmaps) {
    card_bitmaps_atlas = init_card_atlas(find_font(FONT_LIMELIGHT));
    texture_atlas_write(&card_bitmaps_atlas, "cards.png", "cards.tco");
    load_atlas(ATLAS_CARDS, "cards.png", "cards.tco");
  }

#endif // DEBUG

  //
  // setting up game
  //
  init_deck();

  debug.test_game.players_count = 4;
  start_game(&debug.test_game);

  set_button_ids();
  init_guis();

  init_game_draw(&debug.test_game, &game_draw);

  return SUCCESS;
}

internal void
play_destroy() {

}

internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
    scene->view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
    scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

    ortho_scene->view = identity_m4x4();
    ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}

internal void
draw_fps() {
  float_to_string((float32)app_time.frames_per_s, global_buffer);
  set_draw_font(FONT_ROBOTO_MONO);
  draw_text(global_buffer.str(), {0, 0}, 20, {255, 0, 0, 1});
}

internal void
test_3D() {
  gfx_default_viewport();
  gfx_scissor_push({0, 0}, {(float32)gfx.window.dim.width, (float32)gfx.window.dim.height});
  vulkan_depth_test(true);

  scene.view = get_view(camera);
  gfx_ubo(GFXID_SCENE, &scene.view, 0);

  gfx_bind_pipeline(PIPELINE_3D);

  Material_Shader m_s = {};
  gfx_ubo(GFXID_MATERIAL, &m_s, 0);
  gfx_bind_bitmap(GFXID_TEXTURE, BITMAP_LANA, 0);

  draw_rect_3D({0, 0, 0}, {10, 10, 10}, {255, 0, 0, 1});

  Local local = {};
  local.color = {0, 255, 0, 1};
  gfx_ubo(GFXID_LOCAL, &local, 0);

  Object object = {};
  object.model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {1.0f, 0.1f, 1.0f});
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  draw_geometry(find_geometry(GEOMETRY_CARD));

  gfx_scissor_pop();
}

s32 draw() {
  switch(gfx_start_frame()) {
    case GFX_SKIP_FRAME: return SUCCESS;
    case GFX_ERROR:      return FAILURE;
    case GFX_DO_FRAME:   break;
  }

  vulkan_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
  update_scenes(&scene, &ortho_scene, gfx.window.dim);

  if (game_draw.enabled) {
    draw_game(&debug.test_game);
  }

  texture_atlas_refresh(&find_font(FONT_LIMELIGHT)->cache->atlas);
  texture_atlas_refresh(&find_font(FONT_ROBOTO_MONO)->cache->atlas);

  start_draw_2D();

  set_draw_font(FONT_LIMELIGHT);

  s32 game_should_quit = draw_top_gui();

  draw_fps();
  
  if (game_draw.enabled) {
    for (u32 i = 0; i < 3; i++) {
      float_to_string(camera.position.E[i], global_buffer);
      set_draw_font(FONT_ROBOTO_MONO);
      draw_text(global_buffer.str(), {i * 100.0f, 100}, 20, {255, 0, 0, 1});
    }
  }

  end_draw_2D();
  
  vulkan_end_frame();

  return game_should_quit;
}

internal void
free_fly_update_camera(Camera *camera) {
  float64 mouse_move_speed = 0.1f;
  update_camera_with_mouse(camera, app_input.mouse.relative_coords, mouse_move_speed, mouse_move_speed);
  update_camera_target(camera);    
  float32 m_per_s = 6.0f; 
  float32 m_moved = m_per_s * (float32)app_time.frame_time_s;
  Vector3 move_vector = {m_moved, m_moved, m_moved};
  update_camera_with_keys(camera, move_vector,
                          is_down(app_input.forward), is_down(app_input.backward),
                          is_down(app_input.left),    is_down(app_input.right),
                          is_down(app_input.up),      is_down(app_input.down));
}

s32 update() {
  if (on_down(app_input.refresh_shaders)) {
    for (u32 i = 0; i < assets.pipelines.count; i++) {
      Pipeline *pipeline = find_pipeline(i);
      vulkan_pipeline_cleanup(pipeline);
    }
    
    s32 load_pipelines_result = load_pipelines();
    if (load_pipelines_result == FAILURE) {
      return FAILURE;
    }
    init_pipelines();
  }

  bool8 update_game = game_draw.enabled && gui_manager.indices.empty();
  if (update_game) {
    free_fly_update_camera(&camera);
  }

  return draw();
}
