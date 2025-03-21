#ifndef ASSETS_H
#define ASSETS_H

/*
../assets/bitmaps/test.png

file_path = ../assets/bitmaps/test.png
file_name = test.png
folder_path = ../assets/bitmaps/
*/

struct File {
    const char *filepath;
    u32 filepath_length;
    const char *path;
    char *ch; // for functions like file_get_char()
    u32 size;
    void *memory;
};

//
// Audio
//

enum Audio_Types {
    AUDIO_TYPE_SOUND_EFFECT,
    AUDIO_TYPE_MUSIC
};

struct Audio {
    u8 *buffer;
    u32 length; // in bytes

    s32 channels;
    s32 sample_rate; // frequency (22050 Hz)
    s32 samples;
};

struct Playing_Audio {
    u8 *position;
    u32 length_remaining;
    u32 type;
};

struct Audio_Player {
    bool8 playing;
    Playing_Audio playing_audios[10];
    u32 playing_audios_count;

    u8 *buffer;
    u32 length;     // in bytes largest amount copied

    u32 bytes_queued_last_frame;
    s32 bytes_queued;
    
    u32 max_length; // in bytes amount availabe

    SDL_AudioDeviceID device_id;

    float32 music_volume = 0.0f;
    float32 sound_effects_volume = 0.0f;
};

//
// Vextex
//

typedef enum {
    VECTOR2,
    VECTOR3
} Vector_Type;

#ifdef VULKAN
inline VkFormat
convert_to_vulkan(Vector_Type type) {
    switch(type) {
        case VECTOR2: return VK_FORMAT_R32G32_SFLOAT;
        case VECTOR3: return VK_FORMAT_R32G32B32_SFLOAT;
        default: {
            logprint("convert_to_vulkan()", "not a valid vector type\n");
            ASSERT(0);
            return VK_FORMAT_R32G32B32_SFLOAT;
        } break;
    }
}
#endif // VULKAN

struct Vertex_Info {
    static const u32 max_attributes = 5;
    u32 attributes_count;
    Vector_Type formats[max_attributes];
    u32 offsets[max_attributes];

    u32 size;

    void add(Vector_Type format, u32 offset) {
        if (attributes_count >= max_attributes) {
            logprint("Vertex_Info add()", "tried to add too many attributes\n");
            return;
        }

        formats[attributes_count] = format;
        offsets[attributes_count] = offset;
        attributes_count++;
    }
};

struct Vertex_XNU {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

inline Vertex_Info
get_vertex_xnu_info() {
    Vertex_Info basic_info = {};
    basic_info.add(VECTOR3, offsetof(Vertex_XNU, position));
    basic_info.add(VECTOR3, offsetof(Vertex_XNU, normal));
    basic_info.add(VECTOR2, offsetof(Vertex_XNU, uv));
    basic_info.size = sizeof(Vertex_XNU);
    return basic_info;
}

struct Vertex_XU {
    Vector2 position;
    Vector2 uv;
};

inline Vertex_Info
get_vertex_xu_info() {
    Vertex_Info basic_info = {};
    basic_info.add(VECTOR2, offsetof(Vertex_XU, position));
    basic_info.add(VECTOR2, offsetof(Vertex_XU, uv));
    basic_info.size = sizeof(Vertex_XU);
    return basic_info;
}

//
// render
//

enum shader_stages {
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_TESSELLATION_CONTROL,
    SHADER_STAGE_TESSELLATION_EVALUATION,
    SHADER_STAGE_GEOMETRY,
    SHADER_STAGE_FRAGMENT,
    SHADER_STAGE_COMPUTE,

    SHADER_STAGES_AMOUNT
};

enum descriptor_types {
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
    DESCRIPTOR_TYPE_SAMPLER,
    DESCRIPTOR_TYPE_STORAGE_BUFFER,

    DESCRIPTOR_TYPES_AMOUNT
};

/*
Uniform Shader Structure
Scopes/Update frequency

Globals: Projection / View matrices - memory set once 

Instances: Images (sometimes) Material instance - makes sense - memory set once but used a lot

Locals: Models one per object (can't push images) - memory changes a lot but is used a lot
*/

// Could be descriptor
struct Layout_Binding {
    u32 binding;
    u32 descriptor_type;
    u32 descriptor_count;

    u32 stages[SHADER_STAGES_AMOUNT];
    u32 stages_count;
    
    // ubo specific
    u32 size;

    Layout_Binding() {}

    Layout_Binding(u32 in_binding, u32 in_type, u32 in_stage, u32 in_descriptor_count) {
        binding = in_binding;
        descriptor_type = in_type;
        descriptor_count = in_descriptor_count; 
        stages[0] = in_stage;
        stages_count = 1;
    }

    Layout_Binding(u32 in_binding, u32 in_type, u32 in_stage, u32 in_descriptor_count, u32 in_size) {
        binding = in_binding;
        descriptor_type = in_type;
        descriptor_count = in_descriptor_count;
        stages[0] = in_stage;
        stages_count = 1;
        size = in_size;
    }
};

// What is returned from renderer for the drawing code to interact with the descriptors
struct Descriptor {
    Layout_Binding binding; // layout binding for this binding
    u32 offset; // offset in memory for a single binding in descriptor set
    u32 set_number;
    u32 texture_index; // where to write next texture for this frame

#if VULKAN
    VkDescriptorSet *vulkan_set; // points to a descriptor set in a Layout
#elif OPENGL
    u32 *handle;
    Bitmap **bitmaps;
    u32 *bitmaps_saved;
#endif
};

struct Layout {
    static const u32 max_bindings = 4;
    static const u32 max_sets = 64;

    u32 id;

    bool8 resetted;
    u32 sets_reset_amount; // Descriptors got before layout is reseted the first time won't get overwritten
    u32 sets_in_use;

    Layout_Binding bindings[max_bindings]; // array_index != binding location
    u32 bindings_count;

    u32 set_number; // what set in the shader it is
    u32 offsets[max_sets]; // for static uniform buffers, correlates with descriptor_sets

#if VULKAN
    VkDescriptorSet descriptor_sets[max_sets]; // descriptor / set of descriptors
    VkDescriptorSetLayout descriptor_set_layout; // what is in the set
#elif OPENGL
    u32 handles[max_sets];
    Bitmap *bitmaps[max_sets][16];
    u32 bitmaps_saved[max_sets];
#endif    

    void add_binding(Layout_Binding new_binding) {
        bindings[bindings_count++] = new_binding;
    }

    void reset() {
        if (!resetted) {
            sets_reset_amount = sets_in_use;
            resetted = true;
        }
        sets_in_use = sets_reset_amount;
    }
};

struct Push_Constant {
    u32 shader_stage;
    u32 size;
};

struct Layout_Set {
    Layout *layouts[5];
    Push_Constant push_constants[5];

    u32 layouts_count;
    u32 push_constants_count;

    void add_layout(Layout *layout) {
        layouts[layouts_count++] = layout;
    }

    void add_push(u32 shader_stage, u32 size) {
        Push_Constant push = {};  
        push.shader_stage = shader_stage;
        push.size = size;
        push_constants[push_constants_count++] = push;
    }
};

struct Render_Pipeline {
    bool8 compute = FALSE;
    bool8 blend;
    bool8 depth_test = TRUE;
    bool8 wireframe;
    bool8 compiled = FALSE;

#ifdef VULKAN
    VkPipelineLayout layout;
    VkPipeline handle;
#endif
};

typedef Render_Pipeline GFX_Pipeline;

struct Shader {
    File files[SHADER_STAGES_AMOUNT];       // GLSL
    File spirv_files[SHADER_STAGES_AMOUNT]; // SPIRV

    Vertex_Info vertex_info;
    Layout_Set set; // layout of the descriptors in the shader
    GFX_Pipeline pipeline;
};

//
// Bitmap
//

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

void bitmap_convert_channels(Bitmap *bitmap, u32 new_channels);

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
    void *handle;
    Descriptor desc;
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

//
// Font
//

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
    void *info; // stbtt_fontinfo
    
    Vector2_s32 bb_0; // font bounding box coord 0
    Vector2_s32 bb_1; // font bounding box coord 1

    Font_Cache *cache;
};

//
// Mesh
//

struct Material {
    Vector3 ambient;           // Ka (in .obj files)
    Vector3 diffuse;           // Kd
    Vector3 specular;          // Ks
    float32 specular_exponent; // Ns
    
    Bitmap ambient_map;
    Bitmap diffuse_map;    // map_Kd
    
    const char *id; // id from mtl file
};

struct Mesh {
    Vertex_Info vertex_info;
    void *vertices;
    u32 vertices_count;

    u32 *indices;
    u32 indices_count;

    Material material;

    void *gpu_info;
};

struct Model {
    Mesh *meshes;
    u32 meshes_count;

    Shader *color_shader;
    Shader *texture_shader;
};

#endif // ASSETS_H
