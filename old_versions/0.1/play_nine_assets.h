enum Asset_Bitmap_IDs {
 
  BITMAP_START = ASSET_TYPE_AMOUNT,
  
  BITMAP_BACK,
  BITMAP_ICON,
  BITMAP_FRONT,
  BITMAP_FRONT0,
  BITMAP_FRONT11,
  BITMAP_FRONT12,
  BITMAP_BOT,
  BITMAP_LOADING,

  BITMAP_CARD0,
  BITMAP_CARD1,
  BITMAP_CARD2,
  BITMAP_CARD3,
  BITMAP_CARD4,
  BITMAP_CARD5,
  BITMAP_CARD6,
  BITMAP_CARD7,
  BITMAP_CARD8,
  BITMAP_CARD9,
  BITMAP_CARD10,
  BITMAP_CARD11,
  BITMAP_CARD12,
  BITMAP_CARD13,

  BITMAP_COUNT,
  
};

enum Asset_Shader_IDs {

  SHADER_START = BITMAP_COUNT,
  
  SHADER_BASIC3D,
  SHADER_COLOR3D,
  SHADER_TEXT,
  SHADER_COLOR,
  SHADER_TEXTURE,
  SHADER_TEXT3D,
  SHADER_RAY,
  SHADER_PROMPT,
  
  SHADER_COUNT,
  
};

enum Asset_Sound_IDs {
  
  SOUND_START = SHADER_COUNT,
  SOUND_WOOSH,
  SOUND_TAP,
  SOUND_COUNT,
  
};

enum Asset_Music_IDs {

  MUSIC_START = SHADER_COUNT,
  MUSIC_MORNING,
  MUSIC_COUNT,
};

enum Asset_Font_IDs {
  FONT_START = MUSIC_COUNT,
  FONT_CASLON,
  FONT_COUNT,
};

enum Asset_Model_IDs {

  MODEL_START = FONT_COUNT,
  MODEL_CARD,
  MODEL_TABLE,
  MODEL_COUNT,
};

enum Asset_Texture_Atlas_IDs {

  ATLAS_START = MODEL_COUNT,
  ATLAS_KEYBOARD,
  ATLAS_XBOX,
  ATLAS_COUNT,

};

Asset_Decl assets_decl[] = {

  { ASSET_TYPE_SHADER, NULL},
  { SHADER_BASIC3D, "basic.vert", NULL, NULL, NULL, "basic.frag" },
  { SHADER_COLOR3D, "basic.vert", NULL, NULL, NULL, "color3D.frag" },
  { SHADER_TEXT, "2D.vert", NULL, NULL, NULL, "text.frag" },
  { SHADER_COLOR, "2D.vert", NULL, NULL, NULL, "color.frag" },
  { SHADER_TEXTURE, "2D.vert", NULL, NULL, NULL, "texture.frag" },
  { SHADER_TEXT3D, "basic.vert", NULL, NULL, NULL, "text.frag" },
  { SHADER_RAY, "ray.comp" },
  { SHADER_PROMPT, "2D.vert", NULL, NULL, NULL, "prompt.frag" },

  { ASSET_TYPE_BITMAP, NULL },
  { BITMAP_BACK, "back.png" },
  { BITMAP_ICON, "favicon.png" },
  { BITMAP_FRONT, "front.png" },
  { BITMAP_FRONT0, "front0.png" },
  { BITMAP_FRONT11, "front11.png" },
  { BITMAP_FRONT12, "front12.png" },
  { BITMAP_BOT, "bot.png" },
  { BITMAP_LOADING, "loading.png" },
  { BITMAP_CARD0, "card0.png" },
  { BITMAP_CARD1, "card1.png" },
  { BITMAP_CARD2, "card2.png" },
  { BITMAP_CARD3, "card3.png" },
  { BITMAP_CARD4, "card4.png" },
  { BITMAP_CARD5, "card5.png" },
  { BITMAP_CARD6, "card6.png" },
  { BITMAP_CARD7, "card7.png" },
  { BITMAP_CARD8, "card8.png" },
  { BITMAP_CARD9, "card9.png" },
  { BITMAP_CARD10, "card10.png" },
  { BITMAP_CARD11, "card11.png" },
  { BITMAP_CARD12, "card12.png" },
  { BITMAP_CARD13, "card13.png" },

  { ASSET_TYPE_AUDIO, NULL },
  { SOUND_WOOSH, "woosh.ogg" },
  { SOUND_TAP, "tap.ogg" },

  { MUSIC_MORNING, "Morning.ogg" },

  { ASSET_TYPE_FONT, NULL },
  { FONT_CASLON, "LibreCaslonCondensed-Regular.ttf" },

  { ASSET_TYPE_MODEL, NULL },
  { MODEL_CARD, "/card/card.obj" },
  { MODEL_TABLE, "/table/table.obj" },

  { ASSET_TYPE_ATLAS, NULL },
  { ATLAS_KEYBOARD, "keyboard.png", "keyboard.tco" },
  { ATLAS_XBOX, "xbox_series.png", "xbox_series.tco" },
  
};
