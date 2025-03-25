/*
  File is a buffer but with a filepath
*/
struct File {
  String path;
  void *memory;
  u32 size;
};

/*
  Vertex Type
*/

typedef enum {
  VECTOR2,
  VECTOR3,
} Vector_Type;

struct Vertex_Attribute {
  Vector_Type format;
  u32 offset;
};

struct Vertex_Info {
  static const u32 max_attributes = 5;
  u32 attributes_count;

  Vertex_Attribute attributes[max_attributes];

  u32 size; // total size of all attributes in a struct

  void add(Vector_Type format, u32 offset) {
    if (attributes_count >= max_attributes) {
      printf("Vertex_Info add(): tried to add too many attributes\n");
      return;
    }

    Vertex_Attribute new_attribute = {};
    new_attribute.format = format;
    new_attribute.offset = offset;

    attributes[attributes_count++] = new_attribute;
  }
};

struct Vertex_XU {
  Vector2 position;
  Vector2 uv;

  static Vertex_Info get_vertex_info() {
    Vertex_Info info = {};
    info.add(VECTOR2, offsetof(Vertex_XU, position));
    info.add(VECTOR2, offsetof(Vertex_XU, uv));
    info.size = sizeof(Vertex_XU);
    return info;
  }
};

/*
  Shader
*/

const char *shader_file_types[6][2] = {
  {".vs",      ".vert" },
  {".tcs",     ".cont" },
  {".tes",     ".eval" },
  {".gs",      ".geo"  },
  {".fs",      ".frag" },
  {".compute", ".comp" }
};

// lines up with enum shader_stages
const u32 shaderc_glsl_file_types[] = { 
  shaderc_glsl_vertex_shader,
  shaderc_glsl_tess_control_shader ,
  shaderc_glsl_tess_evaluation_shader,
  shaderc_glsl_geometry_shader,
  shaderc_glsl_fragment_shader,
  shaderc_glsl_compute_shader,
};

struct Shader_File {
  const char *filename;
  u32 loaded;
  u32 stage;
  File glsl;
  File spirv;
};

struct Shader {
  Shader_File files[SHADER_STAGES_COUNT]; // stores filename and all intermediate files

  #ifdef GFX_VULKAN

  VkPipelineLayout layout;
  VkPipeline handle;

  #endif //VULKAN

  // flags for on create graphics pipeline
  bool8 compute    = FALSE; // is a compute shader
  bool8 blend      = FALSE;
  bool8 depth_test = TRUE;
  bool8 wireframe  = FALSE;
  bool8 compiled   = FALSE;

  Vertex_Info vertex_info;

  GFX_Layout_Set set;
};

typedef Shader Pipeline;

struct Mesh {
  Vertex_Info vertex_info;

  void *vertices;
  u32 vertices_count;

  u32 *indices;
  u32 indices_count;

  void *gpu_info;
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

struct Font_Char {
    u32 codepoint; // ascii
    u32 glyph_index; // unicode
    
    s32 ax; // advance width
    s32 lsb; // left side bearing

    Vector2_s32 bb_0; // bounding box coord 0
    Vector2_s32 bb_1; // bounding box coord 1
    // WARNING: Scale this up at draw time does not look good. ax and lsb have to be scaled then though.
};

struct Font_Char_Bitmap {
    Font_Char *font_char;
    float32 scale; // scale factor
    Bitmap bitmap;
    u32 index;

    Vector2_s32 bb_0; // bounding box coord 0 
    Vector2_s32 bb_1; // bounding box coord 1
    // WARNING: inverted from Font_Char bounding box (because of stb_truetype). bb_0 is top left and bb_1 is bottom right in this case.
};

struct Font_GFX {
    bool8 in_use;
    
    Descriptor desc;
    float32 scale;
};

struct Font_Cache {
    s32 font_chars_cached;
    s32 bitmaps_cached;
    s32 gfxs_cached;
    
    Font_Char font_chars[255];
    Font_Char_Bitmap bitmaps[1000];
    Font_GFX gfxs[5];

    //Texture_Atlas atlas;
};

struct Font {
    File file;
    void* info; // stbtt_fontinfo
    
    Vector2_s32 bb_0; // font bounding box coord 0
    Vector2_s32 bb_1; // font bounding box coord 1

    Font_Cache *cache;
};

/*
  Asset Manager
*/

enum Asset_Types {
  AT_SHADER,
  AT_BITMAP,
  AT_FONT,
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
  "../assets/fonts/"
};

const u32 asset_size[] = {
  sizeof(Shader), sizeof(Bitmap), sizeof(Font)
};

struct Asset_Array {
  Buffer buffer;
  u32 count;
  u32 type_size;

  void* find(u32 id) {
    return (void*)((char*)buffer.memory + (id * type_size));
  }
};

void prepare_asset_array(Asset_Array *arr, u32 count, u32 size) {
  if (arr->count == 0) {
    arr->count = count;
    arr->type_size = size;
    arr->buffer = blank_buffer(arr->count * size);
  } else {
    arr->buffer.clear();
  }
}

struct Assets {
  Asset_Array pipelines;
  Asset_Array fonts;
  Asset_Array bitmaps;
};

internal void bitmap_convert_channels(Bitmap *bitmap, u32 new_channels);