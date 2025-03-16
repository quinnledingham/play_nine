Mesh get_rect_mesh_2D() {
  Mesh mesh = {};
  mesh.vertices_count = 4;
  
  mesh.vertices = ARRAY_MALLOC(Vertex_XU, mesh.vertices_count);
  Vertex_XU *vertices = (Vertex_XU *)mesh.vertices;

  vertices[0] = Vertex_XU{ {-0.5, -0.5 }, {0, 0} };
  vertices[1] = Vertex_XU{ {-0.5,  0.5 }, {0, 1} };
  vertices[2] = Vertex_XU{ { 0.5, -0.5 }, {1, 0} };
  vertices[3] = Vertex_XU{ { 0.5,  0.5 }, {1, 1} };

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

  mesh.vertex_info = Vertex_XU::get_vertex_info();
  vulkan_init_mesh(gfx.vk_ctx, &mesh);

  return mesh;
}

void draw_rect(Vector2 coords, Vector2 size, Vector4 color) {

  gfx.bind_pipeline(PIPELINE_2D);

  Descriptor scene_desc = gfx.descriptor(GFXID_SCENE);
  vulkan_update_ubo(gfx.vk_ctx, scene_desc, &ortho_scene);
  vulkan_bind_descriptor_set(gfx.vk_ctx, scene_desc);

  Descriptor color_desc = gfx.descriptor(GFXID_COLOR_2D);
  vulkan_update_ubo(gfx.vk_ctx, color_desc, (void *)&color);
  vulkan_bind_descriptor_set(gfx.vk_ctx, color_desc);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  vulkan_push_constants(gfx.vk_ctx, SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(gfx.vk_ctx, &draw_context.square);
}

//static const int test_b = 1;
//void (*draw_rect_2)(Vector2 coords, Vector2 size, Vector4 color) = test_b ? draw_rect : bababooey;


void init_draw() {
  draw_context.square = get_rect_mesh_2D();
}

/*

init
start_draw
draw_rect, circle
draw_text
end_draw

*/
