struct Shapes {

};

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