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

struct Descriptor {
    u32 binding;
    u32 type;
    u32 stages[SHADER_STAGES_AMOUNT];
    u32 stages_count;
    
    u32 size;
    u32 offsets[2];
    void *handle; // OpenGL

    Descriptor() {
        
    }

    Descriptor(u32 in_binding, u32 in_type, u32 in_stage) {
        binding = in_binding;
        type = in_type;
        stages[0] = in_stage;
        stages_count = 1;
    } 
};

// use these to manage the uniforms in the game code
struct Descriptor_Set {
    u32 set_index; // in shader: set = 0, set = 1
    static const u32 max_descriptors = 10;
    Descriptor descriptors[max_descriptors];
    u32 descriptors_count;

    void *gpu_info;
};

struct Descriptor_Set_2 {
    VkDescriptorSet sets[2]; // 2 for max frames in flight
    u32 offsets[2];          // offsets in memory for each set
    u32 size;
    u32 binding;
};

struct Shader {
    File files[SHADER_STAGES_AMOUNT];       // GLSL
    File spirv_files[SHADER_STAGES_AMOUNT]; // SPIRV
    
    
    static const u32 layout_count = 2; // 2 because there are 2 sets (2 different layouts)
    void *descriptor_layout[layout_count];
    void *descriptor_pool[layout_count]; 
    // gpu_info is not set, they just contaain the layout specification

    // meant to be more of a layout tool
    Descriptor_Set descriptor_sets[layout_count]; // information about the uniforms and samplers

    // where to use descriptor sets for
    static const u32 max_sets = 4;
    u32 sets_count[layout_count];
    VkDescriptorSet sets[layout_count][max_sets]; // 2 for max frames in flight

    Descriptor_Set_2 sets_2[layout_count][2];

    bool8 compiled;
    u32 handle;
};

struct Vertex_XNU {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

struct Mesh {
    Vertex_XNU *vertices;
    u32 vertices_count;

    u32 *indices;
    u32 indices_count;

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

#endif // ASSETS_H