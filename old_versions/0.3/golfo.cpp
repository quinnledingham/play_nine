internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
  //scene->view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
  scene->projection = perspective_projection2(camera.fov, (float32)window_dim.height / (float32)window_dim.width, 0.1f, 1000.0f);
  //scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

  ortho_scene->view = identity_m4x4();
  ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}

internal s32
golfo_load_assets() {
  Assets_Load_Info infos[] = {
    pipeline_info,
    textures_info,
    fonts_info
  };

  const u32 count = ARRAY_COUNT(infos);

  SDL_Thread *asset_threads[count];
  for (u32 i = 0; i < count; i++) {
    asset_threads[i] = SDL_CreateThread(load_assets, "AssetThread", &infos[i]);
  }

  for (u32 i = 0; i < count; i++) {
    if (sdl_wait_thread(asset_threads[i])) {
      return FAILURE;
    }
  }

  return SUCCESS;
}

internal u32
golfo_init() {
  gfx_define_layouts();

  golfo_load_assets();
  init_assets();

  init_draw();

  init_guis();

  create_noise_texture(&menu_noise);

  return 0;
}

internal u32
golfo_update() {
  update_scenes(&scene, &ortho_scene, vk_ctx.window_dim);
  update_noise_texture(&menu_noise);

  vulkan_start_frame();

  Global_Shader global_shader = {};
  //global_shader.time.x = (float32)app_time.run_time_s;
  //global_shader.resolution.xy = cv2(gfx.window.resolution);
  gfx_ubo(GFXID_GLOBAL, &global_shader, 0);

  start_draw_2D();

  texture_atlas_refresh(&find_font(FONT_LIMELIGHT)->cache->atlas);

  s32 game_should_quit = draw_top_gui();

  Texture *lana = find_texture(TEXTURE_LANA);
  draw_rect({ 0, 0 }, { 100, 100 }, lana);
  vulkan_end_frame();

  return game_should_quit;
}