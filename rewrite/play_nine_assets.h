struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

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

  BITMAP_COUNT
};

Asset_Load bitmap_loads[] = {
  { BITMAP_LANA, {"lana.jpeg"} },
};

enum Atlas_Ids {
  ATLAS_KEYBOARD,

  ATLAS_COUNT
};

Asset_Load atlas_loads[] = {
  { ATLAS_KEYBOARD, {"keyboard.png", "keyboard.tco"} },
};

/*
  Pipelines
*/

enum Pipeline_Ids {
  PIPELINE_2D,

  PIPELINE_COUNT
};

Asset_Load pipeline_loads[] = {
  { PIPELINE_2D, {"2D.vs", "2D.fs"} },
};

enum GFX_Layout_IDs {
  GFXID_SCENE,
  GFXID_LOCAL,

  GFXID_COUNT
};

internal void
gfx_define_layouts() {
  gfx.layouts_count = GFXID_COUNT;
  gfx.layouts = ARRAY_MALLOC(GFX_Layout, gfx.layouts_count);
  memset(gfx.layouts, 0, sizeof(GFX_Layout) * gfx.layouts_count);

  gfx.layouts[GFXID_SCENE].set_number = 0;
  gfx.layouts[GFXID_LOCAL].set_number = 1;

  gfx.layouts[GFXID_SCENE].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_VERTEX, 1, sizeof(Scene)); // 2D.vert
  gfx.layouts[GFXID_LOCAL].add_binding(0, DESCRIPTOR_TYPE_UNIFORM_BUFFER, SHADER_STAGE_FRAGMENT, 1, sizeof(Local)); // color.frag, text.frag
  gfx.layouts[GFXID_LOCAL].add_binding(1, DESCRIPTOR_TYPE_SAMPLER, SHADER_STAGE_FRAGMENT, 1, 0); // texture.frag

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
  }
}