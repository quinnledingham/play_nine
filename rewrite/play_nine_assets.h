struct Pipeline_Load {
  u32 id;
  const char *filenames[5];
};

enum Pipeline_Ids {
  SIMPLE_PIPELINE,
  PIPELINE_2D,

  PIPELINE_COUNT
};

Pipeline_Load pipeline_loads[] = {
  { SIMPLE_PIPELINE, {"simple.vs", "simple.fs"} },
  { PIPELINE_2D, {"2D.vs", "color.fs"} },
};

enum Font_Ids {
  FONT_LIMELIGHT,

  FONT_COUNT
};

Pipeline_Load font_loads[] = {
  { FONT_LIMELIGHT, {"Limelight-Regular.ttf"} },
};