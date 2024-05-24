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
    	logprint("save_file", "Cannot open file %s\n", filepath);
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
    u64 i = (file->ch - start);
    if (i >= file->size) {
        //logprint("file_get_char()", "returned EOF using size checking\n");
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

inline s32
file_previous_char(File *file) {
    if (file->ch == 0) 
        file->ch = (char*)file->memory;
    char *ptr = file->ch - 2;
    return *ptr;
}

inline s32
file_peek_char(File *file) {
    const char *ptr = file->ch;
    return *ptr;
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
load_bitmap(File file, bool8 flip_on_load) {
    if (flip_on_load) stbi_set_flip_vertically_on_load(true);
    else              stbi_set_flip_vertically_on_load(false);

    Bitmap bitmap = {};
    bitmap.channels = 4;
    
    // 4 arg always get filled in with the original amount of channels the image had.
    // Currently forcing it to have 4 channels.
    unsigned char *data = stbi_load_from_memory((stbi_uc const *)file.memory, file.size, &bitmap.width, &bitmap.height, 0, bitmap.channels);
    //unsigned char *data = stbi_load(filename, &bitmap.width, &bitmap.height, 0, bitmap.channels);
    u32 data_size = bitmap.width * bitmap.height * bitmap.channels;
    bitmap.memory = (u8 *)platform_malloc(data_size);
    platform_memory_copy(bitmap.memory, data, data_size);
    stbi_image_free(data);

    if (bitmap.memory == 0) 
        logprint("load_bitmap()", "could not load bitmap %s\n", file.path);

    bitmap.pitch = bitmap.width * bitmap.channels;
    bitmap.mip_levels = (u32)floor(log2f((float32)max(bitmap.width, bitmap.height))) + 1;

    return bitmap;
}

void write_bitmap_func(void *context, void *data, int size) {
    File *file = (File *)context;

    file->memory = platform_malloc(size);
    platform_memory_copy(file->memory, data, size);
    
    file->size = size;
}

internal void
write_bitmap(Bitmap *bitmap, File *file) {
    stbi_write_png_to_func(write_bitmap_func, file, bitmap->width, bitmap->height, bitmap->channels, bitmap->memory, bitmap->pitch);
}

internal void
write_bitmap(Bitmap *bitmap, const char *file_path) {
    stbi_write_png(file_path, bitmap->width, bitmap->height, bitmap->channels, bitmap->memory, bitmap->pitch);
}

internal void
free_bitmap(Bitmap bitmap) {
    platform_free(bitmap.memory);
}


//
// Shader
//

// lines up with enum shader_stages
const u32 shaderc_glsl_file_types[6] = { 
    shaderc_glsl_vertex_shader,
    shaderc_glsl_tess_control_shader ,
    shaderc_glsl_tess_evaluation_shader,
    shaderc_glsl_geometry_shader,
    shaderc_glsl_fragment_shader,
    shaderc_glsl_compute_shader,
};

/*
shaderc_compile_options_set_target_env(options, shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
*/
internal File
compile_glsl_to_spirv(File *file, shaderc_compiler_t compiler, u32 shader_kind, shaderc_compile_options_t options) {
    const shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, (char*)file->memory, file->size, (shaderc_shader_kind)shader_kind, get_filename(file->filepath), "main", options);

    u32 num_of_warnings = (u32)shaderc_result_get_num_warnings(result);
    u32 num_of_errors = (u32)shaderc_result_get_num_errors(result);

    if (num_of_warnings != 0 || num_of_errors != 0) {
        const char *error_message = shaderc_result_get_error_message(result);
        logprint("compile_glsl_to_spirv()", "%s\n", error_message);
    }

    u32 length = (u32)shaderc_result_get_length(result);
    const char *bytes = shaderc_result_get_bytes(result);

    File result_file = {};
    result_file.memory = platform_malloc(length);
    platform_memory_set(result_file.memory, 0, length);
    platform_memory_copy(result_file.memory, (void *)bytes, length);
    //result_file.memory = (void*)bytes;
    result_file.size = length;

    shaderc_result_release(result);

    return result_file;
}


internal void
spirv_compile_shader(Shader *shader) {
    shaderc_compiler_t compiler = shaderc_compiler_initialize();
    shaderc_compile_options_t options = shaderc_compile_options_initialize();

    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory == 0) 
            continue; // file was not loaded
    
        shader->spirv_files[i] = compile_glsl_to_spirv(&shader->files[i], compiler, shaderc_glsl_file_types[i], options);
        if (shader->spirv_files[i].size == 0) {
            print("compile_shader() could not compile %s\n", shader->files[i].filepath); 
            return;
        }
    }

    shaderc_compile_options_release(options);
    shaderc_compiler_release(compiler);
}

void spirv_cross_error_callback(void *userdata, const char *error) {
    print("spvc error: %s\n", error);
}

internal File
compile_spirv_to_glsl(File *file) {
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
    spvc_compiler_options_set_uint(options, SPVC_COMPILER_OPTION_GLSL_VERSION, 450);
    spvc_compiler_options_set_bool(options, SPVC_COMPILER_OPTION_GLSL_ES, SPVC_FALSE);
    spvc_compiler_install_compiler_options(compiler_glsl, options);

    spvc_compiler_compile(compiler_glsl, &result);
    //print("Cross-compiled source: %s\n", result);

    File result_file = {};
    result_file.size = 2000;
    result_file.memory = platform_malloc(result_file.size);
    platform_memory_set(result_file.memory, 0, result_file.size);
    u32 result_length = get_length(result);
    if (result_length >= result_file.size)
        logprint("compile_spv_to_glsl()", "glsl bigger than file (%d)\n", result_length);
    platform_memory_copy(result_file.memory, (void*)result, result_length);

    spvc_context_destroy(context);

    return result_file;
}

// loads the files
void load_shader(Shader *shader)
{
    if (shader->files[0].filepath == 0 && shader->files[5].filepath == 0) {
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

internal void
clean_shader(Shader *shader) {
    for(u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory == 0) 
            continue;

        platform_free(shader->files[i].memory);
        shader->files[i].memory = 0;
        platform_free(shader->spirv_files[i].memory);
    }
}

//
// Font
//

internal void
init_font(Font *font, File file) {
    font->info = platform_malloc(sizeof(stbtt_fontinfo));
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    *info = {};

    font->cache = (Font_Cache *)platform_malloc(sizeof(Font_Cache));
    platform_memory_set(font->cache, 0, sizeof(Font_Cache));
    platform_memory_set(font->cache->bitmaps, 0, sizeof(Font_Char_Bitmap) * ARRAY_COUNT(font->cache->bitmaps));

    stbtt_InitFont(info, (u8*)file.memory, stbtt_GetFontOffsetForIndex((u8*)file.memory, 0));
    stbtt_GetFontBoundingBox(info, &font->bb_0.x, &font->bb_0.y, &font->bb_1.x, &font->bb_1.y);
}

internal Font_Char *
load_font_char(Font *font, u32 codepoint) {
	stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->cache->font_chars_cached; i++) {
        Font_Char *font_char = &font->cache->font_chars[i];
        if (font_char->codepoint == codepoint)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->cache->font_chars[font->cache->font_chars_cached++];
    if (font->cache->font_chars_cached >= ARRAY_COUNT(font->cache->font_chars)) 
        font->cache->font_chars_cached = 0;

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
    for (s32 i = 0; i < font->cache->bitmaps_cached; i++) {
        Font_Char_Bitmap *bitmap = &font->cache->bitmaps[i];
        if (bitmap->font_char->codepoint == codepoint && bitmap->scale == scale)
            return bitmap;
    }

    // where to cache new font char
    Font_Char_Bitmap *char_bitmap = &font->cache->bitmaps[font->cache->bitmaps_cached++];
    if (font->cache->bitmaps_cached >= ARRAY_COUNT(font->cache->bitmaps)) 
        font->cache->bitmaps_cached = 0;

    // free bitmap if one is being overwritten
    if (char_bitmap->scale != 0) { 
        stbtt_FreeBitmap(char_bitmap->bitmap.memory, info->userdata);
        render_delete_texture(&char_bitmap->bitmap);
        char_bitmap->bitmap.memory = 0;
    }

    memset(char_bitmap, 0, sizeof(Font_Char_Bitmap));
    char_bitmap->font_char = load_font_char(font, codepoint);
    char_bitmap->scale = scale;

    char_bitmap->bitmap.memory = stbtt_GetGlyphBitmapSubpixel(info, 0, char_bitmap->scale, 0, 0, char_bitmap->font_char->glyph_index, &char_bitmap->bitmap.width, &char_bitmap->bitmap.height, 0, 0);
    char_bitmap->bitmap.channels = 1;
    char_bitmap->bitmap.pitch = char_bitmap->bitmap.width * char_bitmap->bitmap.channels;

    stbtt_GetGlyphBitmapBox(info, char_bitmap->font_char->glyph_index, char_bitmap->scale, char_bitmap->scale, &char_bitmap->bb_0.x, &char_bitmap->bb_0.y, &char_bitmap->bb_1.x, &char_bitmap->bb_1.y);

    if (char_bitmap->bitmap.width != 0) {
        render_create_texture(&char_bitmap->bitmap, TEXTURE_PARAMETERS_CHAR);
    }

    return char_bitmap;
}

internal void
clear_font_bitmap_cache(Font *font) {
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    for (s32 i = 0; i < font->cache->bitmaps_cached; i++) {
        stbtt_FreeBitmap(font->cache->bitmaps[i].bitmap.memory, info->userdata);
        font->cache->bitmaps[i].bitmap.memory = 0;
        render_delete_texture(&font->cache->bitmaps[i].bitmap);
        platform_memory_set(&font->cache->bitmaps[i], 0, sizeof(Font_Char_Bitmap));
    }

    font->cache->bitmaps_cached = 0;
}

float32 get_scale_for_pixel_height(void *info, float32 pixel_height) {
    return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)info, pixel_height);
}

s32 get_codepoint_kern_advance(void *info, s32 ch1, s32 ch2) {
    return stbtt_GetCodepointKernAdvance((stbtt_fontinfo*)info, ch1, ch2);
}

/*
(0, 0)
 -> ###############################
    #   #                         #
    #   #                         #
    #   #                         #  
    #   #                         #
    ###############################
    #   #                         #
    ############################### 
*/
struct String_Draw_Info {
    Vector2 dim;
    Vector2 baseline;
};

// if length != -1 than the dim only includes chars up to the length position
internal String_Draw_Info
get_string_draw_info(Font *font, const char *string, s32 length, float32 pixel_height) {
    String_Draw_Info info = {};    

    if (string == 0) {
        return info;
    } else if (font == 0) {
        logprint("get_string_draw_info()", "no font\n");
        return info;
    }

    stbtt_fontinfo *font_info = (stbtt_fontinfo*)font->info;
    float32 scale = stbtt_ScaleForPixelHeight(font_info, pixel_height);
    u32 index = 0;

    s32 top = 0;
    s32 bottom = 0;
    float32 left = 0;

    while (string[index] != 0) {
        if (length != -1) {
            // if a length is set then only include in the dim the chars up to that point
            if (index == length) break;
        }

        Font_Char_Bitmap *font_char_bitmap = load_font_char_bitmap(font, string[index], scale);
        Font_Char *font_char = font_char_bitmap->font_char;

        float32 char_coords_x = info.dim.x + (font_char->lsb * scale);
        if (char_coords_x < 0.0f)
            left = char_coords_x;

        if (top > font_char_bitmap->bb_0.y)
            top = font_char_bitmap->bb_0.y;

        if (bottom < font_char_bitmap->bb_1.y)
            bottom = font_char_bitmap->bb_1.y;
        
        if ((float32)-font_char_bitmap->bb_0.y > info.baseline.y)
            info.baseline.y = (float32)-font_char_bitmap->bb_0.y;

        int kern = stbtt_GetCodepointKernAdvance(font_info, string[index], string[index + 1]);
        if (string[index + 1])
            info.dim.x += scale * float32(kern + font_char->ax);
        else
            info.dim.x += float32(font_char_bitmap->bb_1.x);
        index++;
    }

    info.baseline.x = -left;
    info.dim.x = float32(info.dim.x - left);
    info.dim.y = float32(bottom - top);
        
    return info;
}

//
// Model
//

void init_model(Model *model) {
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
        Mesh *mesh = &model->meshes[mesh_index];
        render_init_mesh(mesh);
        //platform_free(mesh->vertices);
        //platform_free(mesh->indices);
        if (mesh->material.diffuse_map.memory != 0)
            render_create_texture(&mesh->material.diffuse_map, TEXTURE_PARAMETERS_DEFAULT);
            //free_bitmap(mesh->material.diffuse_map);
    }
}

internal void
clean_model(Model *model) {
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
        Mesh *mesh = &model->meshes[mesh_index];
        platform_free(mesh->vertices);
        platform_free(mesh->indices);
        if (mesh->material.diffuse_map.memory != 0)
            free_bitmap(mesh->material.diffuse_map);
    }
}

//
// Audio
//

internal void
print_audio(Audio audio) {
    print("Loaded Audio:\nChannels: %d\nSamples: %d\nSample Rate: %d\nLength: %d\n", audio.channels, audio.samples, audio.sample_rate, audio.length);
}

internal Audio
load_ogg(File file) {
    Audio audio = {};

    audio.samples = stb_vorbis_decode_memory((const unsigned char *)file.memory, file.size, &audio.channels, &audio.sample_rate, (short **)&audio.buffer);
    if (audio.samples < 0) {
        logprint("load_ogg()", "failed to load audio (%s)\n", file.filepath);
        return audio;
    }
    
    audio.length = audio.samples * 4; // 2 bytes (s16) * 2 channels

    print_audio(audio);
    
    return audio;
}

#ifdef SDL

internal Audio
load_wav(const char *file_name) {
    Audio audio = {};

    SDL_AudioSpec spec = {};
    SDL_LoadWAV(file_name, &spec, &audio.buffer, &audio.length);
    audio.samples = spec.samples;
    audio.channels = spec.channels;
    audio.sample_rate = spec.freq;
   
    print_audio(audio);
    
    return audio;
}

internal bool8
init_audio_player(Audio_Player *player) {
    for (s32 i = 0; i < SDL_GetNumAudioDrivers(); i++) {
        print("Audio driver %d: %s\n", i, SDL_GetAudioDriver(i));
    }

    const char *driver_name = SDL_GetCurrentAudioDriver();
    if (driver_name) {
        print("Audio subsystem initialized: %s.\n", driver_name);
    } else {
        logprint("init_audio_player()", "Audio subsystem not initialized.\n");   
        return true;
    }

    u32 num_audio_devices = SDL_GetNumAudioDevices(0);
    for (u32 i = 0; i < num_audio_devices; i++) {
        print("Audio device %d: %s\n", i, SDL_GetAudioDeviceName(i, 0));
    }

    SDL_AudioSpec desired;
    desired.freq = 22050;
    desired.samples = 1024;
    desired.format = AUDIO_S16;
    desired.callback = 0;
    
    SDL_AudioSpec obtained;

    SDL_AudioSpec device_spec;
    char *device_name = 0;
    //SDL_GetDefaultAudioInfo(&device_name, &device_spec, 0);
    player->device_id = SDL_OpenAudioDevice(device_name, 0, &desired, &obtained, 0);
/*
    if (device_name) {
        print("Audio device selected: %s\n", device_name);
    } else {
        logprint("init_audio_player()", "Audio device not selected.\n");
        return true;
    }
*/
    SDL_PauseAudioDevice(player->device_id, 0);

    player->max_length = 10000;
    player->bytes_queued = 0;
    player->buffer = (u8*)platform_malloc(player->max_length);
    platform_memory_set(player->buffer, 0, player->max_length);

    player->music_volume = 0.5f;
    player->sound_effects_volume = 0.5f;

    platform_memory_set(player->playing_audios, 0, 10 * sizeof(Playing_Audio)); 

    return false;
}

internal void
queue_audio(Audio_Player *player) {
    if (player->length > 0) {
        //print("queueing: %d\n", player->length);
        if (SDL_QueueAudio(player->device_id, player->buffer, player->length))
            print("%s\n", SDL_GetError());
        platform_memory_set(player->buffer, 0, player->max_length); 
    }
}

#endif // SDL

internal s32
play_audio(Audio_Player *player, Audio *audio, u32 type) {
    Playing_Audio *playing_audio;
    s32 index = -1;
    for (u32 i = 0; i < ARRAY_COUNT(player->playing_audios); i++) {
        if (player->playing_audios[i].length_remaining <= 0) {
            playing_audio = &player->playing_audios[i];
            index = i;
            break;
        }
    }

    if (!playing_audio) {
        logprint("play_audio()", "Too many audios playing\n");
        return index;
    }

    playing_audio->position = audio->buffer;
    playing_audio->length_remaining = audio->length;
    playing_audio->type = type;

    return index;
}

internal void
play_sound(const char *tag) {
    Audio *audio = find_audio(global_assets, tag);
    play_audio(&app.player, audio, AUDIO_TYPE_SOUND_EFFECT); 
}

internal void
play_music(const char *tag) {
    Audio *audio = find_audio(global_assets, tag);
    play_audio(&app.player, audio, AUDIO_TYPE_MUSIC); 
}

internal void
mix_audio(Audio_Player *player, float32 frame_time_s) {
    float32 samples_per_second = 22050.0f; // frequency
    u32 bytes_padding = 100;
    u32 sample_count = (u32)floor(frame_time_s * samples_per_second); // samples played last frame
    u32 bytes_per_sample = 4;
    u32 bytes_played_last_frame = sample_count * bytes_per_sample; // 2 bytes per 2 channels for each sample
    
    player->length = 0;

    player->bytes_queued += (player->bytes_queued_last_frame - bytes_played_last_frame);
    if (player->bytes_queued < 0)
        player->bytes_queued = 0;
    player->bytes_queued_last_frame = 0;

    u32 queued = SDL_GetQueuedAudioSize(player->device_id); 
    //print("queued: %d, sdl: %d, sample_count %d\n", player->bytes_queued, queued, bytes_played_last_frame);
    if (queued > 5000) // don't queue more than 5000 bytes
        return;
    
    if (bytes_played_last_frame > player->max_length) {
        logprint("mix_audio()", "Buffer not big enough for %d bytes\n", bytes_played_last_frame);
        bytes_played_last_frame = player->max_length;
        return;
    }
    
    for (u32 i = 0; i < ARRAY_COUNT(player->playing_audios); i++) {
        Playing_Audio *playing = &player->playing_audios[i];
        if (playing->length_remaining <= 0)
            continue;

        u32 bytes_to_copy = bytes_played_last_frame + bytes_padding;
        
        if (playing->length_remaining < bytes_to_copy)
            bytes_to_copy = playing->length_remaining;
        
        // Set volume
        float32 volume = 0.5f;
        switch(playing->type) {
            case AUDIO_TYPE_SOUND_EFFECT: volume = player->sound_effects_volume; break;
            case AUDIO_TYPE_MUSIC:        volume = player->music_volume;         break;
        }

        // Mix into buffer
        u32 buffer_index = 0;
        while(buffer_index < bytes_to_copy) {
            s16 *buffer = (s16*)&player->buffer[buffer_index];
            s16 *source = (s16*)&playing->position[buffer_index];
            *buffer += s16((float32)*source * volume);
            buffer_index += 2; // move two bytes
        }

        // Update playing to after what was mixed
        playing->position += bytes_to_copy;
        playing->length_remaining -= bytes_to_copy;

        if (player->length < bytes_to_copy)
            player->length = bytes_to_copy;
    }    

    player->bytes_queued_last_frame = player->length;
}

internal bool8
no_music_playing(Audio_Player *player) {
    bool8 no_music_playing = true;

    for (u32 i = 0; i < ARRAY_COUNT(player->playing_audios); i++) {
        Playing_Audio *playing = &player->playing_audios[i];
        if (playing->length_remaining <= 0)
            continue;

        if (playing->type == AUDIO_TYPE_MUSIC)
            no_music_playing = false;
    }
    
    return no_music_playing;
}
