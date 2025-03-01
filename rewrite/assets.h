struct File {
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

enum shader_stages {
  SHADER_STAGE_VERTEX,
  SHADER_STAGE_TESSELLATION_CONTROL,
  SHADER_STAGE_TESSELLATION_EVALUATION,
  SHADER_STAGE_GEOMETRY,
  SHADER_STAGE_FRAGMENT,
  SHADER_STAGE_COMPUTE,

  SHADER_STAGES_COUNT
};

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

  #ifdef API3D_VULKAN

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

/*
  Asset Manager
*/

enum Asset_Types {
  AT_SHADER,
  AT_BITMAP,
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
};

struct Asset_Array {
  Buffer buffer;
  u32 count;
};

void prepare_asset_array(Asset_Array *arr, u32 count, u32 size) {
  if (arr->count == 0) {
    arr->count = count;
    arr->buffer = blank_buffer(arr->count * size);
  } else {
    arr->buffer.clear();
  }
}

struct Assets {
  Asset_Array pipelines;
};

