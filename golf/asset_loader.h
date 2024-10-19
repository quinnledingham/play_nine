enum Asset_Types {
  ASSET_TYPE_BITMAP,
  ASSET_TYPE_FONT,
  ASSET_TYPE_SHADER,
  ASSET_TYPE_AUDIO,
  ASSET_TYPE_MODEL,
  ASSET_TYPE_ATLAS,

  ASSET_TYPE_COUNT,
};

struct Asset_Load {
  u32 tag;
  union {
    const char *E[5];
    const char *filename;
    const char *compute;
    struct {
      const char *vertex;
      const char *control;
      const char *evaluation;
      const char *geometry;
      const char *fragment;
    };
  };
};

struct Asset_Files {
  File files[5];
  u32 files_count;
};

struct Array {
  void *data;
  u32 element_size;
  u32 size;
  u32 max_size;

  void *get(u32 i) {
    return (char *)data + (element_size * i);
  }
};
#define ARRAY(a) Array array_##a = { a, sizeof(a[0]), ARRAY_COUNT(a), ARRAY_COUNT(a) }

struct Asset {
  u32 type;
  u32 tag;
  File files[5];

  union {
    Bitmap bitmap;
    Font font;
    Model model;
    
    void *memory;
  };
};

typedef Array Asset_Array;
typedef Array Asset_Files_Array;

struct Assets {
  Array loads[ASSET_TYPE_COUNT];

  u32 count;
  Asset *data;
  Asset_Array types[ASSET_TYPE_COUNT];
};
