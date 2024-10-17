#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_image_write.h>
#include <stb_truetype.h>
#include <stb_vorbis.c>

struct File {
  const char *filepath;
  u32 filepath_length;
  const char *path;
  char *ch; // for functions like file_get_char()
  u32 size;
  void *memory;
};

struct Bitmap {
  u8 *memory; // loaded pixels
  union {
    struct {
      s32 width;
      s32 height;
    };
    Vector2_s32 dim;
  };
  s32 pitch;
  s32 channels;
  u32 mip_levels;

  void *gpu_info;
};

struct Font {
  void *info; // stbtt_fontinfo
};

struct Model {
  
};
