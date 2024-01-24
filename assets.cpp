//
// File
//

internal File
load_file(const char *filepath) {
    File result = {};
    
    FILE *in = fopen(filepath, "rb");
    if(in) {
        fseek(in, 0, SEEK_END);
        result.size = ftell(in);
        fseek(in, 0, SEEK_SET);
        
        result.memory = platform_malloc(result.size);
        fread(result.memory, result.size, 1, in);
        fclose(in);
    } else { 
    	logprint("load_file", "Cannot open file %s\n", filepath);
    }
    
    result.filepath = filepath;
    result.ch = (char*)result.memory;
    
    return result;
}

internal void
save_file(File *file, const char *filepath) {
	FILE *in = fopen(filepath, "wb");
    if(in) {
        fwrite(file->memory, file->size, 1, in);
    } else {
    	logprint("save_file", "Cannot open file %s", filepath);
    }
    fclose(in);
}

internal void
free_file(File *file) {
    if (file->memory != 0) platform_free(file->memory);
    *file = {}; // sets file-memory to 0
}

// uses ch in file
inline s32
file_get_char(File *file) {
    char *start = (char *)file->memory;
    if ((file->ch - start) > file->size) {
        logprint("file_get_char()", "returned EOF using size checking\n");
        return EOF;
    }
    
    return *(file->ch++);
}

inline void
file_reset_char(File *file) {
    file->ch = (char *)file->memory;
}

inline void
file_un_char(File *file) {
    file->ch--;
}

internal const char*
file_copy_backwards(File *file, u32 length) {
    char *str = (char *)platform_malloc(length + 1);
    platform_memory_set(str, 0, length + 1);

    const char *ptr = file->ch - length;
    for (u32 i = 0; i < length; i++) {
        s32 ch = *ptr++;
        if (ch == EOF) {
            logprint("file_copy_backwards()", "hit EOF\n");
            break;
        }
        str[i] = ch;
    }

    return str;
}

// returns the file with a 0 at the end of the memory.
// useful if you want to read the file like a string immediately.
internal File
read_file_terminated(const char *filepath) {
    File result = {};
    File file = load_file(filepath);

    result.size = file.size + 1;
    result.filepath = filepath;
    result.memory = platform_malloc(result.size);
    platform_memory_copy(result.memory, file.memory, file.size);

    free_file(&file);

    char *r = (char*)result.memory;
    r[file.size] = 0; // last byte in result.memory
    
    return result;
}

//
// Bitmap
//

internal Bitmap
load_bitmap(const char *filename, bool8 flip_on_load) {
    if (flip_on_load) stbi_set_flip_vertically_on_load(true);
    else              stbi_set_flip_vertically_on_load(false);
    Bitmap bitmap = {};
    bitmap.channels = 4;
    // 4 arg always get filled in with the original amount of channels the image had.
    // Currently forcing it to have 4 channels.
    bitmap.memory = stbi_load(filename, &bitmap.width, &bitmap.height, 0, bitmap.channels);
    
    if (bitmap.memory == 0) logprint("load_bitmap()" "could not load bitmap %s\n", filename);
    bitmap.pitch = bitmap.width * bitmap.channels;
    return bitmap;
}

internal Bitmap
load_bitmap(const char *filename) {
    return load_bitmap(filename, true);
}

internal void
free_bitmap(Bitmap bitmap) {
    stbi_image_free(bitmap.memory);
}

enum Texture_Parameters
{
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

#ifdef OPENGL

internal void
init_bitmap_gpu_handle(Bitmap *bitmap, u32 texture_parameters) {
    GLenum target = GL_TEXTURE_2D;
    
    bitmap->gpu_info = platform_malloc(sizeof(u32));
    glGenTextures(1, (u32*)bitmap->gpu_info);
    glBindTexture(target, *(u32*)bitmap->gpu_info);
    
    GLint internal_format = 0;
    GLenum data_format = 0;
    GLint pixel_unpack_alignment = 0;
    
    switch(bitmap->channels) {
        case 1: {
            internal_format = GL_RED,
            data_format = GL_RED,
            pixel_unpack_alignment = 1; 
        } break;

        case 3: {
            internal_format = GL_RGB;
            data_format = GL_RGB;
            pixel_unpack_alignment = 1; // because RGB is weird case unpack alignment can't be 3
        } break;
        
        case 4: {
            internal_format = GL_SRGB8_ALPHA8; //GL_RGBA no sRGB
            data_format = GL_RGBA;
            pixel_unpack_alignment = 4;
        } break;
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
    glTexImage2D(target, 0, internal_format, bitmap->width, bitmap->height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    glGenerateMipmap(target);
    
    switch(texture_parameters)
    {
        case TEXTURE_PARAMETERS_DEFAULT:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

        case TEXTURE_PARAMETERS_CHAR:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
    
    glBindTexture(target, 0);
}

void opengl_init_bitmap(Descriptor_Set *set, Bitmap *bitmap, u32 binding) { 
    init_bitmap_gpu_handle(bitmap, TEXTURE_PARAMETERS_CHAR); 
    set->descriptors[1].handle = bitmap->gpu_info;
}

internal void
free_bitmap_gpu_handle(Bitmap *bitmap) {
    glDeleteTextures(1, (u32*)bitmap->gpu_info);
}

#endif // OPENGL

#ifdef DX12

internal void
init_bitmap_gpu_handle(Bitmap *bitmap, u32 texture_parameters) {

}

internal void
free_bitmap_gpu_handle(Bitmap *bitmap) {
    
}

#endif // DX12

//
// Shader
//

// lines up with enum shader_stages
const u32 shaderc_glsl_file_types[5] = { 
    shaderc_glsl_vertex_shader,
    shaderc_glsl_tess_control_shader ,
    shaderc_glsl_tess_evaluation_shader ,
    shaderc_glsl_geometry_shader,
    shaderc_glsl_fragment_shader,
};

internal File
compile_glsl_to_spv(shaderc_compiler_t compiler, File *file, shaderc_shader_kind shader_kind) {
    const shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, (char*)file->memory, file->size, shader_kind, get_filename(file->filepath), "main", nullptr);

    u32 num_of_warnings = (u32)shaderc_result_get_num_warnings(result);
    u32 num_of_errors = (u32)shaderc_result_get_num_errors(result);

    if (num_of_warnings != 0 || num_of_errors != 0) {
        const char *error_message = shaderc_result_get_error_message(result);
        logprint("vulkan_load_shader()", "%s", error_message);
    }

    u32 length = (u32)shaderc_result_get_length(result);
    const char *bytes = shaderc_result_get_bytes(result);

    File result_file = {};
    result_file.memory = (void*)bytes;
    result_file.size = length;
    return result_file;
}


internal void
vulkan_compile_shader(Shader *shader) {
    shaderc_compiler_t compiler = shaderc_compiler_initialize();

    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory == 0) continue; // file was not loaded
    
        shader->spirv_files[i] = compile_glsl_to_spv(compiler, &shader->files[i], (shaderc_shader_kind)shaderc_glsl_file_types[i]);
        if (shader->spirv_files[i].size == 0) {
            print("compile_shader() could not compile %s\n", shader->files[i].filepath); 
            return;
        }
    }

}

void spirv_cross_error_callback(void *userdata, const char *error) {
    print("spvc error: %s\n", error);
}

internal File
compile_spv_to_glsl(File *file) {
    spvc_context context = NULL;
    spvc_parsed_ir ir = NULL;
    spvc_compiler compiler_glsl = NULL;
    spvc_compiler_options options = NULL;
    spvc_resources resources = NULL;
    const spvc_reflected_resource *list = NULL;
    const char *result = NULL;
    size_t count;

    spvc_context_create(&context);
    spvc_context_set_error_callback(context, spirv_cross_error_callback, nullptr);
    spvc_context_parse_spirv(context, (SpvId*)file->memory, file->size / 4, &ir);

    spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);

    // Do some basic reflection.
    spvc_compiler_create_shader_resources(compiler_glsl, &resources);
    spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_UNIFORM_BUFFER, &list, &count);

/*
    for (i = 0; i < count; i++)
    {
        printf("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id, list[i].name);
        printf("  Set: %u, Binding: %u\n", spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationDescriptorSet), spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBinding));
    }
*/
    // Modify options.
    spvc_compiler_create_compiler_options(compiler_glsl, &options);
    spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 330);
    spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_FALSE);
    spvc_compiler_install_compiler_options(compiler_glsl, options);

    spvc_compiler_compile(compiler_glsl, &result);
    //print("Cross-compiled source: %s\n", result);

    spvc_context_destroy(context);

    File result_file = {};
    result_file.memory = platform_malloc(1000);
    platform_memory_set(result_file.memory, 0, 1000);
    platform_memory_copy(result_file.memory, (void*)result, get_length(result));
    result_file.size = 0;
    return result_file;
}

// loads the files
void load_shader(Shader *shader)
{
    if (shader->files[0].filepath == 0) {
        logprint("load_shader()", "must have a vertex shader\n");
        return;
    }

    print("loaded shader: ");
    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory != 0) 
            platform_free(&shader->files[i].memory);
        shader->files[i].memory = 0;

        if (shader->files[i].filepath != 0) {
            //shader->files[i] = read_file_terminated(shader->files[i].filepath);
            shader->files[i] = load_file(shader->files[i].filepath);
            print("%s ", shader->files[i].filepath);
        }
    }
    print("\n");
}

//
// Font
//

internal Font
load_font(const char *filepath) {
	Font font = {};
	platform_memory_set(font.font_chars, 0, sizeof(Font_Char)        * ARRAY_COUNT(font.font_chars));
    platform_memory_set(font.bitmaps,    0, sizeof(Font_Char_Bitmap) * ARRAY_COUNT(font.bitmaps));
    font.file = load_file(filepath);
    
    return font;
}

internal void
init_font(Font *font) {
	font->info = platform_malloc(sizeof(stbtt_fontinfo));
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    *info = {};
    
    stbtt_InitFont(info, (u8*)font->file.memory, stbtt_GetFontOffsetForIndex((u8*)font->file.memory, 0));
    stbtt_GetFontBoundingBox(info, &font->bb_0.x, &font->bb_0.y, &font->bb_1.x, &font->bb_1.y);
}

internal Font_Char *
load_font_char(Font *font, u32 codepoint) {
	stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->font_chars_cached; i++) {
        Font_Char *font_char = &font->font_chars[i];
        if (font_char->codepoint == codepoint)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached++];
    if (font->font_chars_cached >= ARRAY_COUNT(font->font_chars)) 
        font->font_chars_cached = 0;

    platform_memory_set(font_char, 0, sizeof(Font_Char));
    font_char->codepoint = codepoint;
    font_char->glyph_index = stbtt_FindGlyphIndex(info, font_char->codepoint);
    
    // how wide is this character
    stbtt_GetGlyphHMetrics(info, font_char->glyph_index, &font_char->ax, &font_char->lsb);
    stbtt_GetGlyphBox(info, font_char->glyph_index, &font_char->bb_0.x, &font_char->bb_0.y, &font_char->bb_1.x, &font_char->bb_1.y);

    return font_char;
}

internal void
vulkan_create_texture(Bitmap *bitmap);

internal Font_Char_Bitmap *
load_font_char_bitmap(Font *font, u32 codepoint, float32 scale) {
    if (scale == 0.0f) {
        logprint("load_font_char_bitmap()", "scale can not be zero"); // scale used below
        return 0;
    }

    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->bitmaps_cached; i++) {
        Font_Char_Bitmap *bitmap = &font->bitmaps[i];
        if (bitmap->font_char->codepoint == codepoint && bitmap->scale == scale)
            return bitmap;
    }

    // where to cache new font char
    Font_Char_Bitmap *char_bitmap = &font->bitmaps[font->bitmaps_cached++];
    if (font->bitmaps_cached >= ARRAY_COUNT(font->bitmaps)) 
        font->bitmaps_cached = 0;

    // free bitmap if one is being overwritten
    if (char_bitmap->scale != 0) { 
        stbtt_FreeBitmap(char_bitmap->bitmap.memory, info->userdata);
        //free_bitmap_gpu_handle(&char_bitmap->bitmap);
        char_bitmap->bitmap.memory = 0;
    }

    memset(char_bitmap, 0, sizeof(Font_Char_Bitmap));
    char_bitmap->font_char = load_font_char(font, codepoint);
    char_bitmap->scale = scale;

    char_bitmap->bitmap.memory = stbtt_GetGlyphBitmapSubpixel(info, 0, char_bitmap->scale, 0, 0, char_bitmap->font_char->glyph_index, &char_bitmap->bitmap.width, &char_bitmap->bitmap.height, 0, 0);
    char_bitmap->bitmap.channels = 1;

    stbtt_GetGlyphBitmapBox(info, char_bitmap->font_char->glyph_index, 0, char_bitmap->scale, &char_bitmap->bb_0.x, &char_bitmap->bb_0.y, &char_bitmap->bb_1.x, &char_bitmap->bb_1.y);

    if (char_bitmap->bitmap.width != 0)
        vulkan_create_texture(&char_bitmap->bitmap);
    //render_init_bitmap(&char_bitmap->bitmap, TEXTURE_PARAMETERS_CHAR);

    return char_bitmap;
}

float32 get_scale_for_pixel_height(void *info, float32 pixel_height) {
    return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)info, pixel_height);
}


s32 get_codepoint_kern_advance(void *info, s32 ch1, s32 ch2) {
    return stbtt_GetCodepointKernAdvance((stbtt_fontinfo*)info, ch1, ch2);
}
