enum Bitmap_IDs {
  BITAMP_BACK,
  BITMAP_ICON,
  BITMAP_FRONT,
  BITMAP_FRONT0,
  BITMAP_FRONT11,
  BITMAP_FRONT12,
  BITMAP_BOT,
  BITMAP_LOADING,

  BITMAP_COUNT
};

enum Sound_IDs {
  SOUND_WOOSH,
  SOUND_TAP,

  SOUND_COUNT
};

enum Music_IDs {
  MUSIC_MORNING,

  MUSIC_COUNT
};

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
  
  { "card0", "card0.png" },
  { "card1", "card1.png" },
  { "card2", "card2.png" },
  { "card3", "card3.png" },
  { "card4", "card4.png" },
  { "card5", "card5.png" },
  { "card6", "card6.png" },
  { "card7", "card7.png" },
  { "card8", "card8.png" },
  { "card9", "card9.png" },
  { "card10", "card10.png" },
  { "card11", "card11.png" },
  { "card12", "card12.png" },
  { "card13", "card13.png" },
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

Asset_Load atlases_to_load[] = {
  { "KEYBOARD_PROMPT", "keyboard.png", "keyboard.tco" },
  { "XBOX_SERIES_PROMPT", "xbox_series.png", "xbox_series.tco" }
};

Assets_Load assets_to_load[] = {
  { ASSET_TYPE_SHADER, ARRAY_COUNT(shaders_to_load), shaders_to_load },
  { ASSET_TYPE_BITMAP, ARRAY_COUNT(bitmaps_to_load), bitmaps_to_load },
  { ASSET_TYPE_FONT, ARRAY_COUNT(fonts_to_load), fonts_to_load },
  { ASSET_TYPE_MODEL, ARRAY_COUNT(models_to_load), models_to_load },
  { ASSET_TYPE_AUDIO, ARRAY_COUNT(audios_to_load), audios_to_load },
  { ASSET_TYPE_ATLAS, ARRAY_COUNT(atlases_to_load), atlases_to_load },
};

Audio_Player *audio_player; // play_sound & play_music

internal void
play_sound(const char *tag) {
    Audio *audio = find_audio(global_assets, tag);
    play_audio(audio_player, audio, AUDIO_TYPE_SOUND_EFFECT); 
}

internal void
play_music(const char *tag) {
    Audio *audio = find_audio(global_assets, tag);
    play_audio(audio_player, audio, AUDIO_TYPE_MUSIC); 
}
