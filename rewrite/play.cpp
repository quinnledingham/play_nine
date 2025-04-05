#include "play.h"

internal void
set_button_ids() {
  set(IN_BACK, SDLK_ESCAPE);

  set(IN_FORWARD, SDLK_W);
  set(IN_FORWARD, SDLK_UP);
  set(IN_BACKWARD, SDLK_S);
  set(IN_BACKWARD, SDLK_DOWN);
  set(IN_LEFT, SDLK_A);
  set(IN_LEFT, SDLK_LEFT);
  set(IN_RIGHT, SDLK_D);
  set(IN_RIGHT, SDLK_RIGHT);
  set(IN_UP, SDLK_SPACE);
  set(IN_DOWN, SDLK_LSHIFT);

  set(IN_REFRESH_SHADERS, SDLK_V);
  set(IN_TOGGLE_CAMERA, SDLK_C);
  set(IN_RESET_GAME, SDLK_K);
  set(IN_TOGGLE_WIREFRAME, SDLK_B);

  set(IN_CARD_0, SDLK_Q);
  set(IN_CARD_1, SDLK_W);
  set(IN_CARD_2, SDLK_E);
  set(IN_CARD_3, SDLK_R);

  set(IN_CARD_4, SDLK_A);
  set(IN_CARD_5, SDLK_S);
  set(IN_CARD_6, SDLK_D);
  set(IN_CARD_7, SDLK_F);

  set(IN_DRAW_PILE, SDLK_1);
  set(IN_DISCARD_PILE, SDLK_2);
  set(IN_PASS, SDLK_P);

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

  set_button_ids();
  init_guis();

  //
  // setting up game
  //
  init_deck();

  start_game(&debug.test_game, 4);

  init_game_draw(&debug.test_game, &game_draw);

  init_animations(animations);

  camera.up = { 0, -1, 0 };
  camera.fov = 75.0f;

  camera.pose = get_player_camera(-game_draw.degrees_between_players, debug.test_game.active_player);

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

internal void
free_fly_update_camera(Camera *camera) {
  float32 mouse_move_speed = 0.1f;
  update_camera_with_mouse(camera, app_input.mouse.relative_coords, mouse_move_speed, mouse_move_speed);
  update_camera_target(camera);    
  float32 m_per_s = 6.0f; 
  float32 m_moved = m_per_s * (float32)app_time.frame_time_s;
  Vector3 move_vector = {m_moved, m_moved, m_moved};
  update_camera_with_keys(camera, move_vector,
                          is_down(IN_FORWARD), is_down(IN_BACKWARD),
                          is_down(IN_LEFT),    is_down(IN_RIGHT),
                          is_down(IN_UP),      is_down(IN_DOWN));
}

internal void
do_keyboard_input(bool8 *input) {
  ASSERT(GI_SIZE == IN_GAME_SIZE);

  for (u32 i = 0; i < GI_SIZE; i++) {
    if (on_down(i)) {
      input[i] = true;
    }
  }
}

internal void
do_mouse_input(Game *game, bool8 *input) {
  bool8 hover[GI_SIZE];
  memset(hover, 0, sizeof(bool8) * GI_SIZE);
  set_ray_coords(&mouse_ray, camera, scene, app_input.mouse.coords, gfx.window.dim);

  for (u32 i = 0; i < HAND_SIZE; i++) {
    Player_Card *card = &game->players[game->active_player].cards[i];
    hover[i] = ray_model_intersection_cpu(mouse_ray, &game_draw.hitbox, card->entity->transform);
  }

  if (on_down(app_input.mouse.left)) {
    memcpy(input, hover, sizeof(bool8) * GI_SIZE);
  } else {
    for (u32 i = 0; i < HAND_SIZE; i++) {
      Player_Card *card = &game->players[game->active_player].cards[i];
      card->entity->hovered = hover[i];
    }
  }
}

internal s32
update_game(Game *game) {

  //
  // Camera
  //
#ifdef DEBUG

  if (on_down(IN_TOGGLE_CAMERA)) {
    debug.free_camera = !debug.free_camera;
    sdl_toggle_relative_mouse_mode();

    if (!debug.free_camera) {
      camera.pose = get_player_camera(-game_draw.degrees_between_players, debug.test_game.active_player);
    }
  }

  if (debug.free_camera) {
    free_fly_update_camera(&camera);
  } else {

    if (on_down(IN_RESET_GAME)) {
      start_game(&debug.test_game, debug.test_game.players_count);
      init_game_draw(&debug.test_game, &game_draw);
      camera.pose = get_player_camera(-game_draw.degrees_between_players, debug.test_game.active_player);
    }

#endif // DEBUG

    //
    // Game Input
    //

    do_animations(animations, app_time.frame_time_s);

    update_camera_target(&camera);

    bool8 input[GI_SIZE];
    memset(input, 0, sizeof(bool8) * GI_SIZE);
    do_keyboard_input(input);

    do_mouse_input(game, input);

    update_game_with_input(game, input);

#ifdef DEBUG
  }
#endif // DEBUG

  return 0;
}

internal s32
reload_shaders() {
  for (u32 i = 0; i < assets.pipelines.count; i++) {
    Pipeline *pipeline = find_pipeline(i);
    vulkan_pipeline_cleanup(pipeline);
  }
  
  s32 load_pipelines_result = load_pipelines();
  if (load_pipelines_result == FAILURE) {
    return FAILURE;
  }
  init_pipelines();

  return SUCCESS;
}

internal s32 
do_game_frame() {
  //
  // Updating
  //

#ifdef DEBUG

  if (on_down(IN_TOGGLE_WIREFRAME)) {
    debug.wireframe = !debug.wireframe;
    return reload_shaders();
  }

  if (on_down(IN_REFRESH_SHADERS)) {
    return reload_shaders();
  }

#endif // DEBUG

  bool8 update_game_flag = game_draw.enabled && gui_manager.indices.empty();
  if (update_game_flag) {
    update_game(&debug.test_game);
  }

  //
  // Drawing
  //
  switch(gfx_start_frame()) {
    case GFX_SKIP_FRAME: return SUCCESS;
    case GFX_ERROR:      return FAILURE;
    case GFX_DO_FRAME:   break;
  }

  Global_Shader global_shader = {};
  global_shader.time.x = (float32)app_time.run_time_s;
  global_shader.resolution.xy = cv2(gfx.window.resolution);
  gfx_ubo(GFXID_GLOBAL, &global_shader, 0);

  vulkan_clear_color({0.1f, 0.1f, 0.1f, 1.0f});
  update_scenes(&scene, &ortho_scene, gfx.window.dim);
  vulkan_depth_test(true);

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
    for (u32 i = 0; i < 6; i++) {
      float_to_string(camera.position.E[i], global_buffer);
      set_draw_font(FONT_ROBOTO_MONO);
      draw_text(global_buffer.str(), {i * 100.0f, 100}, 20, {255, 0, 0, 1});
    }
  }

  end_draw_2D();
  
  vulkan_end_frame();

  return game_should_quit;
}
