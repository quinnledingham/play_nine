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

template<typename T>
struct Stack {
  T *data = 0;
  u32 size;      // number of elements
  u32 type_size; // size of elements
  u32 data_size; // size * type_size = memory allocated

  u32 index;

  Stack(u32 in_size) {
    size = in_size;
    type_size = sizeof(T);
    data_size = type_size * size;
    data = (T*)malloc(data_size);
  }

  ~Stack() {
    free(data);
  }

  void push(T add) {
    data[index++] = add;
    ASSERT(index < size);
  }

  T pop() {
    T ret = data[index - 1];
    index--;
    return ret;
  }

  T top() {
    return data[index - 1];
  }

  u32 get_size() {
    return index;
  }

  bool8 empty() {
    if (index == 0)
      return true;
    else
      return false;
  }
};

struct Rect {
    union {
        struct {
            float32 x;
            float32 y;
        };
        Vector2 coords;
    };

    union {
        struct {
            float32 width;
            float32 height;
        };
        struct {
            float32 w;
            float32 h;
        };
        Vector2 dim;
    };
    float32 rotation; // radians
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

