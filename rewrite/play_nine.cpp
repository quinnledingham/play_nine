#include "play_nine.h"

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
s32 play_nine_init() {
  gfx_define_layouts();

  // asset loading
  s32 load_pipelines_result = load_pipelines();
  if (load_pipelines_result == FAILURE) {
    return FAILURE;
  }

  load_assets(&assets.fonts, font_loads, ARRAY_COUNT(font_loads), AT_FONT);
  load_assets(&assets.bitmaps, bitmap_loads, ARRAY_COUNT(bitmap_loads), AT_BITMAP);
  prepare_asset_array(&assets.mtllibs, 1, sizeof(Material_Library)); // create blank mtllib array

  init_deck();

  test_game.players_count = 4;
  start_game(&test_game);
  
#ifdef DEBUG

  bool8 recreate_input_prompt_atlases = false;
  if (recreate_input_prompt_atlases) {
    Texture_Atlas atlases[PROMPT_COUNT];
    create_input_prompt_atlas(&atlases[PROMPT_KEYBOARD], keyboard_prompts, ARRAY_COUNT(keyboard_prompts), "../libs/xelu/Keyboard & Mouse/Dark/", "_Key_Dark.png", "prompt.png");
    //create_input_prompt_atlas(&atlases[PROMPT_XBOX_SERIES], xbox_prompts, ARRAY_COUNT(xbox_prompts), "../xelu/Xbox Series/XboxSeriesX_", ".png", "xbox_prompt.png");

    texture_atlas_write(&atlases[PROMPT_KEYBOARD], "keyboard.png", "keyboard.tco");
    //texture_atlas_write(&atlases[PROMPT_XBOX_SERIES], "xbox_series.png", "xbox_series.tco");
  }

#endif // DEBUG

  load_assets(&assets.atlases, atlas_loads, ARRAY_COUNT(atlas_loads), AT_ATLAS);
  set_button_ids();
  init_guis();

  File tails_file = load_file("../assets/geometry/tails/tails.obj");
  tails_geo = load_obj(tails_file);
  init_geometry(&tails_geo);

  return SUCCESS;
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
  char fps[20];
  float_to_string(app_time.frames_per_s, fps, 20);
  set_draw_font(FONT_ROBOTO_MONO);
  draw_text(fps, {0, 0}, 20, {255, 0, 0, 1});
}

internal void
draw_game() {
  gfx_default_viewport();
  gfx_scissor_push({0, 0}, {(float32)gfx.window.dim.width, (float32)gfx.window.dim.height});
  vulkan_depth_test(true);

  scene.view = get_view(camera);
  Descriptor_Set scene_desc_set = gfx_descriptor_set(GFXID_SCENE);
  Descriptor scene_desc = gfx_descriptor(&scene_desc_set, 0);
  vulkan_update_ubo(scene_desc, &scene);
  vulkan_bind_descriptor_set(scene_desc_set);

  gfx_bind_pipeline(PIPELINE_3D);

  Material_Shader m_s = {};
  gfx_bind_descriptor_set(GFXID_MATERIAL, &m_s);
  gfx_bind_bitmap(GFXID_TEXTURE, BITMAP_LANA, 0);

  draw_rect_3D({0, 0, 0}, {10, 10, 10}, {255, 0, 0, 1});

  Vector4 color = {0, 255, 0, 1};
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {1.0f, 1.0f, 1.0f});
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  draw_geometry(&tails_geo);

  gfx_scissor_pop();
}

s32 draw() {
  s32 game_should_quit = false;

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

  if (draw_game_flag) {
    draw_game();
  }

  texture_atlas_refresh(&find_font(FONT_LIMELIGHT)->cache->atlas);
  texture_atlas_refresh(&find_font(FONT_ROBOTO_MONO)->cache->atlas);

  start_draw_2D();

  set_draw_font(FONT_LIMELIGHT);

  game_should_quit = draw_top_gui();

  draw_fps();
  
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

  bool8 update_game = draw_game_flag && gui_manager.indices.empty();
  if (update_game) {
    free_fly_update_camera(&camera);
  }

  s32 app_should_quit = draw();

  return app_should_quit;
}
