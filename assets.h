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
	u32 size;
	void *memory;
};

struct Bitmap {
	u8 *memory;
    s32 width;
    s32 height;
    s32 pitch;
    s32 channels;
    
    u32 gpu_handle;
};

enum shader_types {
    VERTEX_SHADER,                  // 0 (shader files array index)
    TESSELLATION_CONTROL_SHADER,    // 1
    TESSELLATION_EVALUATION_SHADER, // 2
    GEOMETRY_SHADER,                // 3
    FRAGMENT_SHADER,                // 4

    SHADER_TYPE_AMOUNT              // 5
};

struct Shader {
    File files[SHADER_TYPE_AMOUNT];

    bool8 compiled;
    bool8 uniform_buffer_objects_generated;
    u32 handle;
};

struct Vertex_XNU {
    Vector3 position;
    Vector3 normal;
    Vector2 uv;
};

struct Triangle_Mesh {
    Vertex_XNU *vertices;
    u32 vertices_count;

    u32 *indices;
    u32 indices_count;

    u32 vertex_array_object; // OpenGL only

#ifdef DX12
    u64 vertex_buffer_object;
    u64 index_buffer_object;
#elif OPENGL
    u32 vertex_buffer_object;
    u32 index_buffer_object;
#endif
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