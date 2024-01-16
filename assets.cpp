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
    	logprint("load_file", "Cannot open file %s", filepath);
    }
    
    result.filepath = filepath;

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
free_file(File *file)
{
    if (file->memory != 0) platform_free(file->memory);
    *file = {}; // sets file-memory to 0
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
    
    glGenTextures(1, &bitmap->gpu_handle);
    glBindTexture(target, bitmap->gpu_handle);
    
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
            internal_format = GL_RGBA;
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

internal void
free_bitmap_gpu_handle(Bitmap *bitmap) {
    glDeleteTextures(1, &bitmap->gpu_handle);
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

// loads the files
void load_shader(Shader *shader)
{
    if (shader->files[0].filepath == 0) {
        logprint("load_shader()", "must have a vertex shader");
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

#ifdef OPENGL

bool compile_shader(u32 handle, const char *file, int type)
{
    u32 shader =  glCreateShader((GLenum)type);
    glShaderSource(shader, 1, &file, NULL);
    glCompileShader(shader);
    
    GLint compiled_shader = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled_shader);  
    if (!compiled_shader) {
        opengl_debug(GL_SHADER, shader);
    } else {
        glAttachShader(handle, shader);
    }
    
    glDeleteShader(shader);
    
    return compiled_shader;
}

// lines up with enum shader_stages
const u32 opengl_shader_file_types[5] = { 
    GL_VERTEX_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER,
};

// compiles the files
void compile_shader(Shader *shader)
{
    shader->uniform_buffer_objects_generated = false;
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->files[0].memory == 0) {
        logprint("compile_shader(Shader *shader)", "vertex shader required");
        return;
    }

    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory == 0) continue; // file was not loaded

        if (!compile_shader(shader->handle, (char*)shader->files[i].memory, opengl_shader_file_types[i])) {
            print("compile_shader() could not compile %s\n", shader->files[i].filepath); 
            return;
        }
    }

    // Link
    glLinkProgram(shader->handle);

    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program) {
        opengl_debug(GL_PROGRAM, shader->handle);
        logprint("compile_shader()", "link failed");
        return;
    }

    shader->compiled = true;
}

u32 use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

#elif VULKAN



#endif // OPENGL

//
// Triangle_Mesh
//

#ifdef OPENGL

void init_gpu_mesh(Triangle_Mesh *mesh) {
	glGenVertexArrays(1, &mesh->vertex_array_object);

    glGenBuffers(1, &mesh->vertex_buffer_object);
    glGenBuffers(1, &mesh->index_buffer_object);
    
    glBindVertexArray(mesh->vertex_array_object);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex_XNU), &mesh->vertices[0], GL_STATIC_DRAW);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0); // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)0);
    glEnableVertexAttribArray(1); // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)offsetof(Vertex_XNU, normal));
    glEnableVertexAttribArray(2); // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)offsetof(Vertex_XNU, uv));
    
    glBindVertexArray(0);
}

void draw_triangle_mesh(Triangle_Mesh *mesh)
{
    glBindVertexArray(mesh->vertex_array_object);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#endif // OPENGL

#ifdef DX12

#endif // DX12

//
// Font
//
/*
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
        free_bitmap_gpu_handle(&char_bitmap->bitmap);
        char_bitmap->bitmap.memory = 0;
    }

    memset(char_bitmap, 0, sizeof(Font_Char_Bitmap));
    char_bitmap->font_char = load_font_char(font, codepoint);
    char_bitmap->scale = scale;

    char_bitmap->bitmap.memory = stbtt_GetGlyphBitmapSubpixel(info, 0, char_bitmap->scale, 0, 0, char_bitmap->font_char->glyph_index, &char_bitmap->bitmap.width, &char_bitmap->bitmap.height, 0, 0);
    char_bitmap->bitmap.channels = 1;

    stbtt_GetGlyphBitmapBox(info, char_bitmap->font_char->glyph_index, 0, char_bitmap->scale, &char_bitmap->bb_0.x, &char_bitmap->bb_0.y, &char_bitmap->bb_1.x, &char_bitmap->bb_1.y);

    init_bitmap_gpu_handle(&char_bitmap->bitmap, TEXTURE_PARAMETERS_CHAR);

    return char_bitmap;
}

float32 get_scale_for_pixel_height(void *info, float32 pixel_height) {
    return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)info, pixel_height);
}
*/