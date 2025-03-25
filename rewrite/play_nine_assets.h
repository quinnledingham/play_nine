struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

enum Pipeline_Ids {
  SIMPLE_PIPELINE,
  PIPELINE_2D,
  PIPELINE_2D_TEXTURE,

  PIPELINE_COUNT
};

Asset_Load pipeline_loads[] = {
  { SIMPLE_PIPELINE, {"simple.vs", "simple.fs"} },
  { PIPELINE_2D, {"2D.vs", "color.fs"} },
  { PIPELINE_2D_TEXTURE, {"2D.vs", "texture.fs"} },
};

enum Font_Ids {
  FONT_LIMELIGHT,

  FONT_COUNT
};

Asset_Load font_loads[] = {
  { FONT_LIMELIGHT, {"Limelight-Regular.ttf"} },
};

enum Bitmap_Ids {
  BITMAP_LANA,

  BITMAP_COUNT
};

Asset_Load bitmap_loads[] = {
  { BITMAP_LANA, {"lana.jpeg"} },
};