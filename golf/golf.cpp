struct App {
  void *data;
};

struct Golf_State {
  DX_Shader shader;
  DX_Pipeline pipeline;
  Mesh mesh;
};

internal void
update(App *app) {
  Golf_State *state = (Golf_State *)app->data;
  dx_start_frame(&gfx.dx12, &state->pipeline);

  dx12_set_viewport(gfx.window_dim.width, gfx.window_dim.height);
  dx12_set_scissor(0, 0, gfx.window_dim.width, gfx.window_dim.height);
  dx12_draw_mesh(&state->mesh);

  dx_end_frame(&gfx.dx12);
}

enum App_Event {
  APP_INIT,
  APP_UPDATE,
  APP_EXIT
};

  
s32 event_handler(App *app, App_Event event) {
  switch(event) {
    case APP_INIT: {
      app->data = malloc(sizeof(Golf_State));
      memset(app->data, 0, sizeof(Golf_State));
      Golf_State *state = (Golf_State *)app->data;
      
      if(dx_compile_shader(&state->shader)) {
        return 1;
      }

      state->pipeline.shader = &state->shader;
      dx_create_pipeline(&gfx.dx12, &state->pipeline);

      Vertex_PC triangle_vertices[] =
      {
          { {   0.0f,  0.25f * gfx.aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
          { {  0.25f, -0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
          { { -0.25f, -0.25f * gfx.aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
      };
      state->mesh.vertices = (void *)triangle_vertices;
      state->mesh.vertices_count = 3;
      state->mesh.vertex_info.attributes = Vector_PC_Attributes;
      state->mesh.vertex_info.attributes_count = 2;
      dx_init_mesh(&gfx.dx12, &state->mesh);
    } break;

    case APP_UPDATE: {
      update(app);
    } break;

    case APP_EXIT: {


    } break;
  }

  return 0;
}
