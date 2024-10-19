struct App {
  void *data;
};

struct Golf_State {
  D3D12_Shader shader;
  D3D12_Pipeline pipeline;
  Mesh mesh;
};

internal void
update(App *app) {
  Golf_State *state = (Golf_State *)app->data;

  d3d12_start_frame();
  d3d12_bind_pipeline(&state->pipeline);
  d3d12_set_viewport(gfx.window_dim.width, gfx.window_dim.height);
  d3d12_set_scissor(0, 0, gfx.window_dim.width, gfx.window_dim.height);
  d3d12_draw_mesh(&state->mesh);

  d3d12_end_frame();
}

enum App_Event {
  APP_INIT,
  APP_UPDATE,
  APP_EXIT
};

Assets assets = {};

s32 event_handler(App *app, App_Event event) {
  switch(event) {
    case APP_INIT: {
      app->data = malloc(sizeof(Golf_State));
      memset(app->data, 0, sizeof(Golf_State));
      Golf_State *state = (Golf_State *)app->data;
      
      if(d3d12_compile_shader(&state->shader)) {
        return 1;
      }

      state->pipeline.shader = &state->shader;
      d3d12_create_pipeline(&gfx.d3d12, &state->pipeline);
      /*
      Vertex_PC triangle_vertices[] =
      {
          { {   0.0f,  0.25f * gfx.aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  0.25f, -0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -0.25f, -0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
      };
      */
     Vertex_PU triangle_vertices[] =
      {
          { { 0.0f,  0.0f  * gfx.aspect_ratio, 0.0f }, { 0.0f, 0.0f } },
          { { 0.0f,  0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 1.0f } },
          { { 0.25f, 0.0f  * gfx.aspect_ratio, 0.0f }, { 1.0f, 0.0f } },
          
          { { 0.25f, 0.0f  * gfx.aspect_ratio, 0.0f }, { 1.0f, 0.0f } },
          { { 0.0f,  0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 1.0f } },
          { { 0.25f, 0.25f * gfx.aspect_ratio, 0.0f }, { 1.0f, 1.0f } },
      };
      state->mesh.vertices = (void *)triangle_vertices;
      state->mesh.vertices_count = 6;
      state->mesh.vertex_info.attributes = Vector_PU_Attributes;
      state->mesh.vertex_info.attributes_count = 2;
      d3d12_init_mesh(&gfx.d3d12, &state->mesh);

      d3d12_start_commands();
      d3d12_bind_pipeline(&state->pipeline);
      assets.loads[ASSET_TYPE_BITMAP] = array_bitmaps;
      load_assets(&assets);
      d3d12_end_commands();
      d3d12_wait_for_gpu(&gfx.d3d12);
    } break;

    case APP_UPDATE: {
      update(app);
    } break;

    case APP_EXIT: {


    } break;
  }

  return 0;
}
