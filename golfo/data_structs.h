// once the size is set it can't be changed
template<typename T>
struct Arr {
  T *data = 0;
  u32 count;
  u32 data_size = 0;

  void init() {
    if (data) {
      destroy();
    }

    data_size = count * sizeof(T);
    data = (T *)SDL_malloc(data_size);
  }

  void destroy() {
    SDL_free(data);
  }

  T& operator [] (int i) { 
    if (i >= (int)count) {
      SDL_Log("WARNING: tried to access memory outside of Arr range. Returned last element instead\n");
      return data[count - 1];
    }

    return data[i]; 
  }
};

/*
  File is a buffer but with a filepath
*/
struct File {
  String path;
  void *memory;
  u32 size;
};

/*
  Texture
*/

struct Texture {
  u8 *data; 
    union {
        struct {
            u32 width;
            u32 height;
        };
        Vector2_u32 dim;
    };
    s32 pitch;
    s32 channels;
    u32 mip_levels = 1;

  // Vulkan
  VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
  VkDeviceMemory memory;  // Could put in vulkan info and save it
  VkImage image;      // similar to VkBuffer
  VkImageView image_view; // provides more info about the image
  VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
  VkSampler sampler;      // allows the shader to sample the image
};

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

  static Vertex_Info info() {
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
  GFX_Layouts
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

enum descriptor_types {
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_STORAGE_IMAGE,
    DESCRIPTOR_TYPE_STORAGE_BUFFER,

    DESCRIPTOR_TYPES_AMOUNT
};

struct GFX_Layout_Binding {
  u32 binding;
  u32 descriptor_type;
  u32 descriptor_count;

  u32 stages[SHADER_STAGES_COUNT];
  u32 stages_count;

  // ubo
  u32 size;
};

struct GFX_Layout {
  static const u32 max_bindings = 4;
  static const u32 max_sets = 512;

  u32 id;

  bool8 resetted;
  u32 sets_reset_amount; // Descriptors got before layout is reseted the first time won't get overwritten
  u32 sets_in_use;

  GFX_Layout_Binding bindings[max_bindings];
  u32 bindings_count;

  u32 set_number; // what set in the shader it is
  u32 offsets[max_sets]; // for static uniform buffers, correlates with descriptor_sets

  VkDescriptorSet descriptor_sets[max_sets]; // descriptor / set of descriptors
  VkDescriptorSetLayout descriptor_set_layout; // what is in the set

  void add_binding(u32 in_binding, u32 in_type, u32 in_stage, u32 in_descriptor_count, u32 in_size) {
    GFX_Layout_Binding *binding = &bindings[bindings_count++];
    binding->binding = in_binding;
    binding->descriptor_type = in_type;
    binding->descriptor_count = in_descriptor_count;
    binding->stages[0] = in_stage;
    binding->stages[1] = SHADER_STAGE_COMPUTE;
    binding->stages_count = 2;
    binding->size = in_size;
  }

  void reset() {
    if (!resetted) {
      sets_reset_amount = sets_in_use;
      resetted = true;
    }
    sets_in_use = sets_reset_amount;
  }
};

struct GFX_Push_Constant {
    u32 shader_stage;
    u32 size;
};

struct GFX_Layout_Set {
  GFX_Layout *layouts[5];
  u32 layouts_count;

  GFX_Push_Constant push_constants[5];
  u32 push_constants_count;

  u32 binded_layouts;

  void add_layout(GFX_Layout *layout) {
    layouts[layouts_count++] = layout;
  }

  void add_push(u32 shader_stage, u32 size) {
    GFX_Push_Constant push = {};  
    push.shader_stage = shader_stage;
    push.size = size;
    push_constants[push_constants_count++] = push;
  }
};

// What is returned from renderer for the drawing code to interact with the descriptors
struct Descriptor_Set {
    //GFX_Layout_Binding binding; // layout binding for this binding
    GFX_Layout *layout;
    u32 offset; // offset in memory for a single binding in descriptor set
    u32 set_number;
    u32 texture_index; // where to write next texture for this frame

    VkDescriptorSet *vulkan_set; // points to a descriptor set in a Layout
};

struct Descriptor {
  Descriptor_Set *set;
  GFX_Layout_Binding *binding; // layout binding for this binding
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

struct Vulkan_Buffer {
  VkBuffer handle;
  VkDeviceMemory memory;
  VkDeviceSize size;
  
  u32 offset; // where to enter new bytes
  void *data; // if the memory is mapped
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