enum Model_Tags {
  MODEL_CARD,
  MODEL_TABLE,

  MODEL_COUNT
};

enum Font_Tags {
  FONT_CASLON,

  FONT_COUNT
};

enum Bitmap_Tags {
  BITMAP_CARD_BACK,

  BITMAP_COUNT
};


Asset_Load models[] = {
  { MODEL_CARD, "/card" },
  { MODEL_TABLE, "/table" }
};

Asset_Load fonts[] = {
  { FONT_CASLON, "LibreCaslonCondensed-Regular" },
};

Asset_Load bitmaps[] = {
  { BITMAP_CARD_BACK, "back" },
};
ARRAY(bitmaps);
