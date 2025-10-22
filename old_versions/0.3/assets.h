#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

/*
  File is a buffer but with a filepath
*/

struct File {
  String path;
  void *memory;
  u32 size;
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
  u32 files_count;

  VkPipelineLayout layout;
  VkPipeline handle;

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


/*
  Buffer
*/

Buffer blank_buffer(u32 size) {
  Buffer buffer = {};

  buffer.size = size;
  buffer.memory = malloc(buffer.size);
  memset(buffer.memory, 0, buffer.size);

  return buffer;
}

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
  
  Texture ambient_map;
  Texture diffuse_map;    // map_Kd

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

/*
  Texture
  @NOTICE: Texture is defined in vulkan_golfo.h
*/

struct Dynamic_Texture {
  Texture src;
  Texture textures[Vulkan_Context::max_frames_in_flight];
  Descriptor_Set sets[Vulkan_Context::max_frames_in_flight];
  bool8 refresh_required[Vulkan_Context::max_frames_in_flight];
};


//
// Font
//

/*
####p2
######
p1####
*/
struct Texture_Coords {
    Vector2 p1;
    Vector2 p2;
};

struct Texture_Atlas {
  bool8 resetted;

  Dynamic_Texture texture;  

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
    Texture bitmap;
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

/*


  Asset Manager



*/

enum Asset_Types {
  AT_SHADER,
  AT_TEXTURE,
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
  sizeof(Texture), 
  sizeof(Font), 
  //sizeof(Texture_Atlas),
  //sizeof(Geometry),
  //sizeof(Material_Library),
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
    app_log_error("assert_array_next(): not enough room to insert\n");
    ASSERT(0);
  }
  return insert_ptr;
}

struct Assets {
  Asset_Array pipelines;
  Asset_Array fonts;
  Asset_Array textures;
  Asset_Array atlases;
  Asset_Array geometrys;
  Asset_Array mtllibs;
};

struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

struct Assets_Load_Info {
    Asset_Array *asset_array;
    Asset_Load *loads;
    u32 loads_count;
    u32 asset_type;
};

internal void texture_convert_channels(Texture *bitmap, u32 new_channels);

#endif // ASSETS_LOADER_H
