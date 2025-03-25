struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

enum Pipeline_Ids {
  PIPELINE_2D,

  PIPELINE_COUNT
};

Asset_Load pipeline_loads[] = {
  { PIPELINE_2D, {"2D.vs", "2D.fs"} },
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