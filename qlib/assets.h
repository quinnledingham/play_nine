#ifndef ASSETS_H
#define ASSETS_H

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

/*
../assets/bitmaps/test.png

file_path = ../assets/bitmaps/test.png
file_name = test.png
folder_path = ../assets/bitmaps/
*/

struct File {
	const char *filepath;
    const char *path;
    char *ch; // for functions like file_get_char()
	u32 size;
	void *memory;
};

struct Bitmap {
	u8 *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
    
    void *gpu_info;
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

    Vector2_s32 bb_0; // bounding box coord 0 
    Vector2_s32 bb_1; // bounding box coord 1
    // WARNING: inverted from Font_Char bounding box (because of stb_truetype). bb_0 is top left and bb_1 is bottom right in this case.
};

struct Font {
    File file;
    void *info; // stbtt_fontinfo
    
    Vector2_s32 bb_0; // font bounding box coord 0
    Vector2_s32 bb_1; // font bounding box coord 1

    s32 font_chars_cached;
    s32 bitmaps_cached;
    
    Font_Char font_chars[255];
    Font_Char_Bitmap bitmaps[300];
};

struct Audio {

};

//
// render
//

enum shader_stages {
    SHADER_STAGE_VERTEX,
    SHADER_STAGE_TESSELLATION_CONTROL,
    SHADER_STAGE_TESSELLATION_EVALUATION,
    SHADER_STAGE_GEOMETRY,
    SHADER_STAGE_FRAGMENT,

    SHADER_STAGES_AMOUNT
};

enum descriptor_types {
    DESCRIPTOR_TYPE_UNIFORM_BUFFER,
    DESCRIPTOR_TYPE_SAMPLER,

    DESCRIPTOR_TYPES_AMOUNT
};

/*
Uniform Shader Structure
Scopes/Update frequency

Globals: Projection / View matrices - memory set once 

Instances: Images (sometimes) Material instance - makes sense - memory set once but used a lot

Locals: Models one per object (can't push images) - memory changes a lot but is used a lot
*/

enum struct descriptor_scope {
    GLOBAL,   // ubo
    INSTANCE, // ubo
    LOCAL,    // push constant

    AMOUNT
};

struct Descriptor {
    u32 binding;
    u32 type;
    u32 stages[SHADER_STAGES_AMOUNT];
    u32 stages_count;
    descriptor_scope scope;
    
    u32 size;
    u32 offsets[2];
    void *handle; // OpenGL

    Descriptor() {
        
    }

    Descriptor(u32 in_binding, u32 in_type, u32 in_stage, u32 in_size, descriptor_scope in_scope) {
        binding = in_binding;
        type = in_type;
        stages[0] = in_stage;
        stages_count = 1;
        size = in_size;
        scope = in_scope;
    } 

    void Push_Descriptor(u32 in_stage, u32 in_size, descriptor_scope in_scope) {
        stages[0] = in_stage;
        stages_count = 1;
        size = in_size;
        scope = in_scope;
        offsets[0] = 0;
        offsets[1] = 0;
    } 
};

// use these to manage the uniforms in the game code
struct Descriptor_Set {
    u32 set_index; // in shader: set = 0, set = 1
    static const u32 max_descriptors = 10;
    Descriptor descriptors[max_descriptors];
    u32 descriptors_count;
    u32 push_constant_count;

#ifdef VULKAN
    VkDescriptorSet *gpu_info[2]; // Vulkan Descriptor Set
#endif
    //u32 handles[2]; // index to descriptor sets (0 - (max_sets - 1))
};

// Contains vulkan info about each descriptor layout
#ifdef VULKAN
struct Vulkan_Shader_Info {
    static const u32 max_sets = 200;
    u32 sets_count;

    VkDescriptorSetLayout descriptor_set_layout;
    VkDescriptorPool descriptor_pool;
    VkDescriptorSet descriptor_sets[max_sets * 2]; // @TODO add MAX_FRAMES_IN_FLIGHT    
};
#endif

struct Shader {
    File files[SHADER_STAGES_AMOUNT];       // GLSL
    File spirv_files[SHADER_STAGES_AMOUNT]; // SPIRV
    
    static const u32 max_sets = 200;
    static const u32 layout_count = 4;                // layout sets for the 3 scopes
    
    Descriptor_Set layout_sets[layout_count];         // meant to be more of a layout tool
    Descriptor_Set descriptor_sets[layout_count][max_sets]; //

#ifdef VULKAN
    Vulkan_Shader_Info vulkan_infos[layout_count];
#endif

    bool8 compiled;
    u32 handle;
};

//
// Mesh
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

//
// Assets Loading
//

enum Asset_Types {
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_FONT,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_AUDIO,
    ASSET_TYPE_MODEL,

    ASSET_TYPE_AMOUNT
};

struct Asset {
    u32 type;
    const char *tag;
    u32 tag_length;
    union {
        Bitmap bitmap;
        Font font;
        Shader shader;
        Audio audio;
        Model model;
        void *memory;
    };  
};

struct Asset_Array {
    Asset *data;       // points to Assets data array
    u32 num_of_assets;
};

struct Assets {
    u32 num_of_assets;

    Asset *data;
    Asset_Array types[ASSET_TYPE_AMOUNT];
};

internal void*
find_asset(Assets *assets, u32 type, const char *tag) {
    for (u32 i = 0; i < assets->types[type].num_of_assets; i++) {
        if (equal(tag, assets->types[type].data[i].tag))
            return &assets->types[type].data[i].memory;
    }
    logprint("find_asset()", "Could not find asset, type: %d, tag: %s\n", type, tag);
    return 0;
}

inline Bitmap* find_bitmap(Assets *assets, const char *tag) { return (Bitmap*) find_asset(assets, ASSET_TYPE_BITMAP, tag); }
inline Font*   find_font  (Assets *assets, const char *tag) { return (Font*)   find_asset(assets, ASSET_TYPE_FONT,   tag); }
inline Shader* find_shader(Assets *assets, const char *tag) { return (Shader*) find_asset(assets, ASSET_TYPE_SHADER, tag); }
inline Model*  find_model (Assets *assets, const char *tag) { return (Model*)  find_asset(assets, ASSET_TYPE_MODEL,  tag); }

#endif // ASSETS_H
