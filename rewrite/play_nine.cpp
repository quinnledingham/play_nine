#include "play_nine.h"

s32 load_pipelines() {
  print("loading pipelines...\n");

  u32 filenames_count = ARRAY_COUNT(pipeline_loads[0].filenames);
  u32 pipeline_loads_count = ARRAY_COUNT(pipeline_loads);

  prepare_asset_array(&assets.pipelines, pipeline_loads_count, sizeof(Pipeline));

  for (u32 i = 0; i < pipeline_loads_count; i++) {
    Pipeline *pipeline = find_pipeline(pipeline_loads[i].id);
    Pipeline_Load *load = &pipeline_loads[i];

    for (u32 file_index = 0; file_index < filenames_count; file_index++) {
      if (!load->filenames[file_index]) {
        continue;
      }

      s32 result = load_shader_file(pipeline, load->filenames[file_index]);
      if (result == FAILURE) {
        return FAILURE;
      }
    }

    spirv_compile_shader(pipeline);
    //gfx.create_graphics_pipeline(pipeline, gfx.draw_render_pass);
  }

  gfx.init();

  for (u32 i = 0; i < pipeline_loads_count; i++) {
    Pipeline *pipeline = find_pipeline(pipeline_loads[i].id);
    gfx.create_graphics_pipeline(pipeline, gfx.draw_render_pass);
  }

  return SUCCESS;
}

/*
  0 = init was successfull
  1 = init failed
*/
s32 play_nine_init() {
  s32 load_pipelines_result = load_pipelines();
  if (load_pipelines_result == FAILURE) {
    return FAILURE;
  }

  square = get_rect_mesh_2D();

  return SUCCESS;
}

void update_scenes(Scene *scene, Scene *ortho_scene, Vector2_s32 window_dim) {
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

  gfx.clear_color({1, 0, 1, 1});
  update_scenes(&scene, &ortho_scene, gfx.window.dim);

  if (gfx.window.resized) {
    gfx.create_frame_resources();
    gfx.window.resized = false;
  }

  if (gfx.start_frame()) {
    return FAILURE;
  }

  gfx.default_viewport();
  gfx.default_scissor();
  gfx.depth_test(false);

  /*
  gfx.bind_pipeline(SIMPLE_PIPELINE);
  vkCmdDraw(*gfx.active_command_buffer, 3, 1, 0, 0);
*/
  gfx.bind_pipeline(PIPELINE_2D);

  Descriptor scene_desc = gfx.descriptor(GFXID_SCENE);
  gfx.update_ubo(scene_desc, &ortho_scene);
  gfx.bind_descriptor_set(scene_desc);

  Vector4 color = {255, 1, 1, 1};
  Descriptor color_desc = gfx.descriptor(GFXID_COLOR_2D);
  gfx.update_ubo(color_desc, (void *)&color);
  gfx.bind_descriptor_set(color_desc);

  Object object = {};
  object.model = create_transform_m4x4({200, 200, 0}, get_rotation(0, {0, 1, 0}), {100, 100, 1});
  object.index = 0;
  gfx.push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  gfx.draw_mesh(&square);

  gfx.end_frame(gfx.resolution_scaling, gfx.window.resized);

  return SUCCESS;
}

s32 update() {

  s32 draw_result = draw();
  if (draw_result == FAILURE) {
    log_error("update(): draw failed\n");
  }

  return SUCCESS;
}
