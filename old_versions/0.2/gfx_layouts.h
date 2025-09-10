//
// GFX Layouts
//

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
