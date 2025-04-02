struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

internal s32 load_assets(Asset_Array *asset_array, Asset_Load *loads, u32 loads_count, u32 asset_type);

enum Font_Ids {
  FONT_LIMELIGHT,
  FONT_ROBOTO_MONO,

  FONT_COUNT
};

Asset_Load font_loads[] = {
  { FONT_LIMELIGHT, {"Limelight-Regular.ttf"} },
  { FONT_ROBOTO_MONO, {"RobotoMono.ttf"} },
};

enum Bitmap_Ids {
  BITMAP_LANA,

  BITMAP_BACK,
  BITMAP_FRONT,
  BITMAP_FRONT_0,
  BITMAP_FRONT_11,
  BITMAP_FRONT_12,

  BITMAP_COUNT
};

Asset_Load bitmap_loads[] = {
  { BITMAP_LANA, {"lana.jpeg"} },

  { BITMAP_BACK, {"back.png"} },
  { BITMAP_FRONT, {"front.png"} },
  { BITMAP_FRONT_0, {"front0.png"} },
  { BITMAP_FRONT_11, {"front11.png"} },
  { BITMAP_FRONT_12, {"front12.png"} },
};

enum Atlas_Ids {
  ATLAS_KEYBOARD,
  ATLAS_CARDS,

  ATLAS_COUNT
};

Asset_Load atlas_loads[] = {
  { ATLAS_KEYBOARD, {"keyboard.png", "keyboard.tco"} },
  { ATLAS_CARDS, {"cards.png", "cards.tco"} },
};

enum Geometry_Ids {
  GEOMETRY_TAILS,
  GEOMETRY_CARD,
  GEOMETRY_CARD_SIDE,

  GEOMETRY_COUNT
};

Asset_Load geometry_loads[] = {
  { GEOMETRY_TAILS, { "tails/tails.obj" } },
  { GEOMETRY_CARD, { "card/card.obj" } },
  { GEOMETRY_CARD_SIDE, { "card/card_side.obj" } },
};

/*
  Pipelines
*/

enum Pipeline_Ids {
  PIPELINE_2D,
  PIPELINE_3D,
  PIPELINE_NOISE,

  PIPELINE_COUNT
};

Asset_Load pipeline_loads[] = {
  { PIPELINE_2D, {"2D.vs", "2D.fs"} },
  { PIPELINE_3D, {"3D.vs", "3D.fs"} },
  { PIPELINE_NOISE, {"2D.vs", "noise.fs"} },
};

enum GFX_Layout_IDs {
  GFXID_SCENE,
  GFXID_LOCAL,
  GFXID_TEXTURE,
  GFXID_MATERIAL,

  GFXID_COUNT
};

internal void
gfx_define_layouts() {
  gfx.layouts_count = GFXID_COUNT;
  gfx.layouts = ARRAY_MALLOC(GFX_Layout, gfx.layouts_count);
  memset(gfx.layouts, 0, sizeof(GFX_Layout) * gfx.layouts_count);

  gfx.layouts[GFXID_SCENE].set_number = 0;
  gfx.layouts[GFXID_LOCAL].set_number = 1;
  gfx.layouts[GFXID_TEXTURE].set_number = 2;
  gfx.layouts[GFXID_MATERIAL].set_number = 3;

  gfx.layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  gfx.layouts[GFXID_LOCAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Local)); // color.frag, text.frag
  gfx.layouts[GFXID_TEXTURE].add_binding(0, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 1, 0); // texture.frag
  gfx.layouts[GFXID_MATERIAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Material_Shader)); // 3D.frag

  for (u32 i = 0; i < gfx.layouts_count; i++) {
    vulkan_setup_layout(&gfx.layouts[i]);
  }
}

internal void 
gfx_add_layouts_to_shaders() {
  {
    Pipeline *shader = find_pipeline(PIPELINE_2D);
    shader->vertex_info = Vertex_XU::get_vertex_info();
    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_3D);
    shader->vertex_info = Vertex_XNU::info();
    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
    shader->set.add_layout(&gfx.layouts[GFXID_MATERIAL]);
  }

  {
    Pipeline *shader = find_pipeline(PIPELINE_NOISE);
    shader->vertex_info = Vertex_XU::get_vertex_info();
    shader->set.add_layout(&gfx.layouts[GFXID_SCENE]);
    shader->set.add_push(SHADER_STAGE_VERTEX, sizeof(Object));

    shader->set.add_layout(&gfx.layouts[GFXID_LOCAL]);
    shader->set.add_layout(&gfx.layouts[GFXID_TEXTURE]);
  }
}