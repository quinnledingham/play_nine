internal void
update_scenes(Scene *scene, Scene *ortho_scene, Vector2_u32 window_dim) {
  //scene->view = look_at({ 2.0f, 2.0f, 2.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f });
  scene->projection = perspective_projection2(camera.fov, (float32)window_dim.height / (float32)window_dim.width, 0.1f, 1000.0f);
  //scene->projection = perspective_projection(45.0f, (float32)window_dim.width / (float32)window_dim.height, 0.1f, 1000.0f);

  ortho_scene->view = identity_m4x4();
  ortho_scene->projection = orthographic_projection(0.0f, (float32)window_dim.width, 0.0f, (float32)window_dim.height, -3.0f, 3.0f);
}


internal u32
golfo_init() {
  gfx_define_layouts();
  load_assets(pipeline_info);
  load_assets(textures_info);
  init_assets();
  init_draw();
  return 0;
}

internal void
gfx_ubo(u32 gfx_id, void *data, u32 binding) {
  Descriptor_Set desc_set = vulkan_descriptor_set(gfx_id);
  Descriptor desc = vulkan_descriptor(&desc_set, binding);
  vulkan_update_ubo(desc, data);
  vulkan_bind_descriptor_set(desc_set);
}

internal u32
golfo_update() {
  update_scenes(&scene, &ortho_scene, vk_ctx.window_dim);

  vulkan_start_frame();

  Global_Shader global_shader = {};
  //global_shader.time.x = (float32)app_time.run_time_s;
  //global_shader.resolution.xy = cv2(gfx.window.resolution);
  gfx_ubo(GFXID_GLOBAL, &global_shader, 0);

  start_draw_2D();
  draw_rect({ 0, 0 }, { 100, 100 }, {255, 0, 0, 1});
  vulkan_end_frame();
  return 0;
}