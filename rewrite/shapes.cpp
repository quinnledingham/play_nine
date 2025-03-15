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
  gfx.init_mesh(&mesh);

  return mesh;
}

void draw_rect(Vector2 coords, Vector2 size, Vector4 color) {
  gfx.bind_pipeline(PIPELINE_2D);

  Descriptor scene_desc = gfx.descriptor(GFXID_SCENE);
  gfx.update_ubo(scene_desc, &ortho_scene);
  gfx.bind_descriptor_set(scene_desc);

  Descriptor color_desc = gfx.descriptor(GFXID_COLOR_2D);
  gfx.update_ubo(color_desc, (void *)&color);
  gfx.bind_descriptor_set(color_desc);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  gfx.push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  gfx.draw_mesh(&shapes.square);
}

void init_shapes() {
  shapes.square = get_rect_mesh_2D();
}
