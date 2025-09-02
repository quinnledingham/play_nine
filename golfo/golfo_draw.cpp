internal Mesh
get_rect_mesh_2D() {
  Mesh mesh = {};
  mesh.vertices_count = 4;
  
  mesh.vertices = ARRAY_MALLOC(Vertex_XU, mesh.vertices_count);
  Vertex_XU *vertices = (Vertex_XU *)mesh.vertices;

  //vertices[0] = Vertex_XU{ {-0.5, -0.5 }, {0, 0} };
  //vertices[1] = Vertex_XU{ {-0.5,  0.5 }, {0, 1} };
  //vertices[2] = Vertex_XU{ { 0.5, -0.5 }, {1, 0} };
  //vertices[3] = Vertex_XU{ { 0.5,  0.5 }, {1, 1} };

  vertices[0] = Vertex_XU{ { 0, 0 }, {0, 0} };
  vertices[1] = Vertex_XU{ { 0, 1 }, {0, 1} };
  vertices[2] = Vertex_XU{ { 1, 0 }, {1, 0} };
  vertices[3] = Vertex_XU{ { 1, 1 }, {1, 1} };

  mesh.indices_count = 6;
  mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

  // vertex locations
  u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
 
  mesh.indices[0] = top_left;
  mesh.indices[1] = bottom_right;
  mesh.indices[2] = bottom_left;
  mesh.indices[3] = top_left;
  mesh.indices[4] = top_right;
  mesh.indices[5] = bottom_right;

  mesh.vertex_info = Vertex_XU::info();
  vulkan_init_mesh(&mesh);

  return mesh;
}

internal void
draw_rect(Vector2 coords, Vector2 size, Vector4 color) {
  Descriptor_Set local_desc_set = vulkan_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = vulkan_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square);
}

internal void
init_draw() {
  draw_ctx.square = get_rect_mesh_2D();
}


internal void
start_draw_2D() {
  vulkan_set_viewport(vk_ctx.window_dim.width, vk_ctx.window_dim.height);
  vulkan_set_scissor(0, 0, vk_ctx.window_dim.width, vk_ctx.window_dim.height);
  //gfx_scissor_push({0, 0}, {(float32)gfx.window.dim.width, (float32)gfx.window.dim.height});
  vulkan_depth_test(false);

  vulkan_bind_pipeline(PIPELINE_2D);

  Descriptor_Set scene_desc_set = vulkan_descriptor_set(GFXID_SCENE);
  Descriptor scene_desc = vulkan_descriptor(&scene_desc_set, 0);
  vulkan_update_ubo(scene_desc, &ortho_scene);
  vulkan_bind_descriptor_set(scene_desc_set);

  // Default texture
  Texture *bitmap = find_texture(TEXTURE_LANA);
  Descriptor_Set texture_desc_set = vulkan_descriptor_set(GFXID_TEXTURE);
  Descriptor texture_desc = vulkan_descriptor(&texture_desc_set, 0);
  vulkan_set_texture(&texture_desc, bitmap);
  vulkan_bind_descriptor_set(texture_desc_set);
}

internal void
end_draw_2D() {
  //gfx_scissor_pop();
}
