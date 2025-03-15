struct Shapes {
  Mesh square;
};

GFX gfx;

Shader test = {};

Shapes shapes = {};

Assets assets = {};

Scene scene;
Scene ortho_scene;

Pipeline* find_pipeline(u32 id) {
  return ((Pipeline *)assets.pipelines.buffer.memory) + id;
}

Font* find_font(u32 id) {
  return ((Font *)assets.fonts.buffer.memory) + id;
}

bool8 local_vulkan_context = false;

// Play Nine Game

s8 deck[DECK_SIZE];
Game test_game;

// Drawing Game

Vector2 hand_coords[HAND_SIZE];
global float32 hand_width;
Vector2 card_dim = { 20.0f, 32.0f };