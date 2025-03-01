//
// GFX Layouts
//

enum descriptor_types {
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_STORAGE_BUFFER,

    DESCRIPTOR_TYPES_AMOUNT
};

enum GFX_Layout_IDs {
  GFXID_SCENE,
  GFXID_COLOR_2D,
  
  GFXID_COUNT
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

// What is returned from renderer for the drawing code to interact with the descriptors
struct Descriptor {
    GFX_Layout_Binding binding; // layout binding for this binding
    u32 offset; // offset in memory for a single binding in descriptor set
    u32 set_number;
    u32 texture_index; // where to write next texture for this frame

    VkDescriptorSet *vulkan_set; // points to a descriptor set in a Layout
};

struct GFX_Layout {
  static const u32 max_bindings = 4;
  static const u32 max_sets = 64;

  u32 id;

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
    binding->stages[0] = in_stage;
    binding->stages_count = 1;
    binding->size = in_size;
  }
};

struct GFX_Layout_Set {
  GFX_Layout *layouts[5];
  u32 layouts_count;
};