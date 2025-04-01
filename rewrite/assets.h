/*
  File is a buffer but with a filepath
*/
struct File {
  String path;
  void *memory;
  u32 size;
};

internal File load_file(const char *filepath);
internal void destroy_file(File *file);

inline bool8
in_file(File *file, char *ptr) {
  s32 diff = ptr - (char*)file->memory;
  return 0 <= diff && diff < file->size;
}

/*
  Vertex Type
*/

typedef enum {
  VECTOR2,
  VECTOR3,
  VECTOR4
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
    //info.add(VECTOR2, offsetof(Vertex_XU, position));
    //info.add(VECTOR2, offsetof(Vertex_XU, uv));
    info.add(VECTOR4, 0);
    info.size = sizeof(Vertex_XU);
    return info;
  }
};

struct Vertex_XNU {
  Vector3 position;
  Vector3 normal;
  Vector2 uv;

  static Vertex_Info info() {
    Vertex_Info info = {};
    info.add(VECTOR3, offsetof(Vertex_XNU, position));
    info.add(VECTOR3, offsetof(Vertex_XNU, normal));
    info.add(VECTOR2, offsetof(Vertex_XNU, uv));
    info.size = sizeof(Vertex_XNU);
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

internal Bitmap load_bitmap(File file);
internal Bitmap load_bitmap_flip(File file);

/*
####p2
######
p1####
*/
struct Texture_Coords {
    Vector2 p1;
    Vector2 p2;
};

struct Texture_Atlas_GPU {
    void *handle; // Vulkan_Texture
    Descriptor_Set set;
    //Descriptor desc;
    bool8 refresh_required;
};

struct Texture_Atlas {
  bool8 resetted;
  Bitmap bitmap;

  Texture_Atlas_GPU gpu[MAX_FRAMES_IN_FLIGHT];

  static const u32 max_textures = 1000;
  u32 texture_count;
  Texture_Coords texture_coords[max_textures]; 

  // variables for adding new textures
  Vector2_s32 insert_position;
  s32 row_height;
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

    Texture_Atlas atlas;
};

struct Font {
  File file;
  void* info; // stbtt_fontinfo
  
  Vector2_s32 bb_0; // font bounding box coord 0
  Vector2_s32 bb_1; // font bounding box coord 1

  Font_Cache *cache;
};

// Shader struct copy
struct Material_Shader {
  Vector4 ambient;             // Ka (in .obj files)
  Vector4 diffuse;             // Kd
  Vector4 specular;            // Ks Ns
};

struct Material {
  String id; // id from mtl file

  Vector3 ambient;             // Ka (in .obj files)
  Vector3 diffuse;             // Kd
  Vector3 specular;            // Ks
  Vector3 emissive_coefficent; // Ke
  float32 optical_density;     // Ni
  float32 dissolve;            // d
  float32 specular_exponent;   // Ns
  u32 illumination;            // illum
  
  Bitmap ambient_map;
  Bitmap diffuse_map;    // map_Kd

  Material_Shader shader() {
    Material_Shader mtl = {};
    mtl.ambient = cv4(ambient);
    mtl.diffuse = cv4(diffuse);
    mtl.specular = cv4(specular);
    mtl.specular.a = specular_exponent;
    return mtl;
  }
};

// translate to a .mtl file
struct Material_Library {
  String name;
  Material *materials;
  u32 materials_count;
};

struct Mesh {
  Vertex_Info vertex_info;

  void *vertices;
  u32 vertices_count;

  u32 *indices;
  u32 indices_count;

  u32 material_id;
  Material *material;

  // Vulkan
  u32 vertices_buffer_offset;
  u32 indices_buffer_offset;
  Vulkan_Buffer *buffer;
};

struct Geometry {
  Array<Material_Library*> mtllibs;

  Mesh *meshes;
  u32 meshes_count;
};

Geometry load_obj(File file);

/*
  Asset Manager
*/

enum Asset_Types {
  AT_SHADER,
  AT_BITMAP,
  AT_FONT,
  AT_ATLAS,
  AT_GEOMETRY,
  AT_MTLLIB,

  AT_COUNT
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
  "../assets/fonts/",
  "../assets/atlases/",
  "../assets/geometry/",
  "", // mtllibs in .obj / geometry
};

const u32 asset_size[] = {
  sizeof(Shader), 
  sizeof(Bitmap), 
  sizeof(Font), 
  sizeof(Texture_Atlas),
  sizeof(Geometry),
  sizeof(Material_Library),
};

struct Asset_Array {
  Buffer buffer;
  u32 count;
  u32 type_size;

  u32 insert_index; // if insert functions are being used

  inline void* find(u32 id) {
    return (void*)((char*)buffer.memory + (id * type_size));
  }
};

internal void 
prepare_asset_array(Asset_Array *arr, u32 count, u32 size) {
  if (arr->count == 0) {
    arr->count = count;
    arr->type_size = size;
    arr->buffer = blank_buffer(arr->count * size);
  } else {
    arr->buffer.clear();
  }
}

internal void
asset_array_resize(Asset_Array *arr) {
  ASSERT(arr->type_size != 0);

  if (arr->count == 0) {
    prepare_asset_array(arr, 1, arr->type_size);
  } else {
    arr->count *= 2;
    buffer_resize(&arr->buffer, arr->count * arr->type_size);
  }
}

/*
DOES NOT WORK CORRECTLY

internal void
assert_array_insert(Asset_Array *arr, void *n) {
  ASSERT(arr->count > arr->insert_index);
  void *insert_ptr = (void*)((char*)arr->buffer.memory + (arr->insert_index * arr->type_size));
  if (arr->buffer.in(insert_ptr)) {
    memcpy(insert_ptr, n, arr->type_size);
    arr->insert_index++;
  }  

  log_error("assert_array_insert(): not enough room to insert\n");
  ASSERT(0);
}
*/

internal void*
asset_array_next(Asset_Array *arr) {
  if (arr->insert_index >= arr->count) {
    asset_array_resize(arr);
  }  

  void *insert_ptr = (void*)((char*)arr->buffer.memory + (arr->insert_index++ * arr->type_size));
  if (!arr->buffer.in(insert_ptr)) {
    log_error("assert_array_next(): not enough room to insert\n");
    ASSERT(0);
  }
  return insert_ptr;
}

struct Assets {
  Asset_Array pipelines;
  Asset_Array fonts;
  Asset_Array bitmaps;
  Asset_Array atlases;
  Asset_Array geometrys;
  Asset_Array mtllibs;
};

internal void bitmap_convert_channels(Bitmap *bitmap, u32 new_channels);