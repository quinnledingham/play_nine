Asset_Load shaders_to_load[] = {
 { "BASIC3D", "basic.vert", NULL, NULL, NULL, "basic.frag", NULL },
 { "COLOR3D", "basic.vert", NULL, NULL, NULL, "color3D.frag", NULL },
 { "TEXT", "2D.vert", NULL, NULL, NULL, "text.frag", NULL },
 { "COLOR", "2D.vert", NULL, NULL, NULL, "color.frag", NULL },
 { "TEXTURE", "2D.vert", NULL, NULL, NULL, "texture.frag", NULL },
 { "TEXT3D", "basic.vert", NULL, NULL, NULL, "text.frag", NULL },
 { "RAY", NULL, NULL, NULL, NULL, NULL, "ray.comp" },
 { "PROMPT", "2D.vert", NULL, NULL, NULL, "prompt.frag", NULL },
};

Asset_Load bitmaps_to_load[] = {
  { "BACK", "back.png" },
  { "ICON", "favicon.png" },
  { "FRONT", "front.png" },
  { "FRONT0", "front0.png" },
  { "FRONT11", "front11.png" },
  { "FRONT12", "front12.png" },
  { "BOT", "bot.png" },
  { "LOADING", "loading.png" },
};

Asset_Load fonts_to_load[] = {
  { "CASLON", "LibreCaslonCondensed-Regular.ttf" },
};

Asset_Load models_to_load[] = {
  { "CARD", "/card/card.obj" },
  { "TABLE", "/table/table.obj" },
};

Asset_Load audios_to_load[] = {
  { "WOOSH", "woosh.ogg" },
  { "TAP", "tap.ogg" },
  { "MORNING", "Morning.ogg" },
};

Assets_Load assets_to_load[] = {
  { ASSET_TYPE_SHADER, ARRAY_COUNT(shaders_to_load), shaders_to_load },
  { ASSET_TYPE_BITMAP, ARRAY_COUNT(bitmaps_to_load), bitmaps_to_load },
  { ASSET_TYPE_FONT, ARRAY_COUNT(fonts_to_load), fonts_to_load },
  { ASSET_TYPE_MODEL, ARRAY_COUNT(models_to_load), models_to_load },
  { ASSET_TYPE_AUDIO, ARRAY_COUNT(audios_to_load), audios_to_load },
};
