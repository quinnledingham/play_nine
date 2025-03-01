GFX gfx;

Shader test = {};
Mesh square = {};

Assets assets = {};

Scene scene;
Scene ortho_scene;

Pipeline* find_pipeline(u32 id) {
  return ((Pipeline *)assets.pipelines.buffer.memory) + id;
}
