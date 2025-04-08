#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>
#include <stb/stb_truetype.h>
#include <stb/stb_vorbis.c>

/*
  File
*/

internal File
load_file(const char *filepath) {
  File result = {};
    
  FILE *in;
  fopen_s(&in, filepath, "rb");
  if(in) {
    fseek(in, 0, SEEK_END);
    result.size = ftell(in);
    fseek(in, 0, SEEK_SET);
        
    result.memory = malloc(result.size + 1);
    memset(result.memory, 0, result.size + 1);
    fread(result.memory, result.size, 1, in);
    fclose(in);
  } else { 
    log_error("load_file(): Cannot open file %s\n", filepath);
  }

  // Save filepath
  result.path = String(filepath);

  return result;
}

internal void
save_file(File in_file, FILE *out_file) {
  fwrite(in_file.memory, in_file.size, 1, out_file);
}

internal void
read_file(File in_file, FILE *out_file) {
  fread(in_file.memory, in_file.size, 1, out_file);
}

internal void 
destroy_file(File *file) {
  if (!file->memory)
    return;

  free(file->memory);
  file->memory = 0;
  file->size = 0;
}

File buffer_to_file(Buffer buffer) {
  File result = {};
  result.memory = buffer.memory;
  result.size = buffer.size;
  return result;
}

Buffer blank_buffer(u32 size) {
  Buffer buffer = {};

  buffer.size = size;
  buffer.memory = malloc(buffer.size);
  memset(buffer.memory, 0, buffer.size);

  return buffer;
}

Buffer buffer_copy_create(u32 size, Buffer copy) {
  Buffer buffer = blank_buffer(size);
  memcpy(buffer.memory, copy.memory, copy.size);
  return buffer;
}

void destroy_buffer(Buffer *buffer) {
  if (!buffer->memory)
    return;

  free(buffer->memory);
  buffer->memory = 0;
  buffer->size = 0;
}



/*
  Shader
*/

/*
  shaderc_compile_options_set_target_env(options, shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
*/

void compile_glsl_to_spirv(Shader_File *file, shaderc_compiler_t compiler, u32 shader_kind, shaderc_compile_options_t options) {
  const char *filename = get_filename(file->filename);
  const shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, (char*)file->glsl.memory, file->glsl.size, (shaderc_shader_kind)shader_kind, filename, "main", options);
  free((void*)filename);

  u32 num_of_warnings = (u32)shaderc_result_get_num_warnings(result);
  u32 num_of_errors = (u32)shaderc_result_get_num_errors(result);

  if (num_of_warnings != 0 || num_of_errors != 0) {
    const char *error_message = shaderc_result_get_error_message(result);
    u32 error_size = str_length(error_message);
    log_error("(%s) compile_glsl_to_spirv(): %s\n", file->filename, error_message);
    ASSERT(0);
  }

  u32 length = (u32)shaderc_result_get_length(result);
  const char *bytes = shaderc_result_get_bytes(result);
  Buffer result_buffer = blank_buffer(length);
  memcpy(result_buffer.memory, (void *)bytes, length);

  shaderc_result_release(result);

  destroy_file(&file->spirv);
  file->spirv = buffer_to_file(result_buffer);

  if (file->spirv.size == 0) {
    log_error("compile_glsl_to_spirv() could not compile %s\n", file->filename); 
    return;
  }
}

void spirv_reflection(Shader_File *file) {
  spvc_context context = NULL;
  spvc_parsed_ir ir = NULL;
  spvc_compiler compiler_glsl = NULL;
  spvc_resources resources = NULL;
  const spvc_reflected_resource *list = NULL;
  const char *result = NULL;
  size_t count;
  size_t i;

  spvc_context_create(&context);
  //spvc_context_set_error_callback(context, &spir_reflection_callback, 0);
  spvc_context_parse_spirv(context, (const SpvId *)file->spirv.memory, file->spirv.size / 4, &ir);
  spvc_context_create_compiler(context, SPVC_BACKEND_GLSL, ir, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler_glsl);

  // Do some basic reflection.
  spvc_compiler_create_shader_resources(compiler_glsl, &resources);
  spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_PUSH_CONSTANT, &list, &count);

  for (i = 0; i < count; i++)
  {
    printf("ID: %u, BaseTypeID: %u, TypeID: %u, Name: %s\n", list[i].id, list[i].base_type_id, list[i].type_id,
           list[i].name);
    printf("  Set: %u, Binding: %u\n",
           spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationDescriptorSet),
           spvc_compiler_get_decoration(compiler_glsl, list[i].id, SpvDecorationBinding));
  }

  spvc_context_destroy(context);
}

void spirv_compile_shader(Shader *shader) {
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  for (u32 i = 0; i < SHADER_STAGES_COUNT; i++) {
    Shader_File *file = &shader->files[i];
    if (!file->loaded) 
      continue; // file was not loaded
    
    compile_glsl_to_spirv(file, compiler, shaderc_glsl_file_types[i], options);
    //spirv_reflection(file);
  }

  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);
}

internal u32 
identify_shader_stage(const char *filename) {
  u32 length = get_length(filename);
  char *ptr = (char *)filename;

  ptr += length; // go to the end of string

  // moving backwards until finding a period
  while(*ptr != '.' && ptr > filename) {
    ptr--;
  }

  // compare with saved types
  for (u32 i = 0; i < ARRAY_COUNT(shader_file_types); i++) {
      const char *ending_1 = shader_file_types[i][0];
      const char *ending_2 = shader_file_types[i][1];

      if (!strcmp(ptr, ending_1) || !strcmp(ptr, ending_2)) {
        return i;
      }
  }

  log_error("identify_shader_stage(): count not find a stage that matched -> %s, returning default\n", filename);

  return 0;
}

internal s32 
load_shader_file(Shader *shader, const char *filename) {
  u32 stage = identify_shader_stage(filename);

  Shader_File *file = &shader->files[stage];

  file->filename = filename;
  file->stage = stage;

  destroy_file(&file->glsl);

  String filepath = String(asset_folders[AT_SHADER], filename);
  file->glsl = load_file(filepath.str());

  // try different file endings if the file could not be found
  if (file->glsl.size == 0) {
    filepath.remove_ending();

    for (u32 ending_i = 0; ending_i < 2; ending_i++) {
        String filepath_ending = String(filepath.str(), shader_file_types[file->stage][ending_i]);
        file->glsl = load_file(filepath_ending.str());

        if (file->glsl.size != 0) {
          print("loaded shader file: %s\n", filepath_ending.str());
          filepath_ending.destroy();
          break;
        }
        filepath_ending.destroy();
    }
  }

  if (file->glsl.size) {
    file->loaded = true;
  }

  filepath.destroy();

  if (file->loaded) {
    return SUCCESS;
  } else {
    return FAILURE;
  }
}

internal s32 
load_pipelines() {
  return load_assets(&assets.pipelines, pipeline_loads, ARRAY_COUNT(pipeline_loads), AT_SHADER);
}

internal s32 
init_pipelines() {
  print("initializing pipelines...\n");
  for (u32 i = 0; i < assets.pipelines.count; i++) {
    Pipeline *pipeline = find_pipeline(i);
    vulkan_create_graphics_pipeline(&vk_ctx, pipeline, vk_ctx.draw_render_pass);
  }

  return SUCCESS;
}

internal void
pipeline_print_filenames(Pipeline *pipe) {
  for (u32 i = 0; i < SHADER_STAGES_COUNT; i++) {
    Shader_File *file = &pipe->files[i];
    if (file->filename) {
      print("%s ", file->filename);
    }
  }
  print("\n");
}

internal void
save_pipeline(u32 id, FILE *file) {
  Pipeline *pipeline = find_pipeline(id);
  fwrite(pipeline, sizeof(Pipeline), 1, file);
  for (u32 i = 0; i < SHADER_STAGES_COUNT; i++) {
    if (pipeline->files[i].loaded) {
      save_file(pipeline->files[i].spirv, file);
    }
  }
}

internal void
load_saved_pipeline(u32 id, FILE *file) {
  Pipeline *pipeline = find_pipeline(id);
  fread(pipeline, sizeof(Pipeline), 1, file);
  for (u32 i = 0; i < SHADER_STAGES_COUNT; i++) {
    if (pipeline->files[i].loaded) {
      read_file(pipeline->files[i].spirv, file);
    }
  }
}

/*
  Bitmap
*/


internal Bitmap
load_bitmap(File file) {
  Bitmap bitmap = {};
  bitmap.channels = 0;

  // 4 arg always get filled in with the original amount of channels the image had.
  // Currently forcing it to have 4 channels.
  unsigned char *data = stbi_load_from_memory((stbi_uc const *)file.memory, file.size, &bitmap.width, &bitmap.height, &bitmap.channels, 0);
  u32 data_size = bitmap.width * bitmap.height * bitmap.channels;
  bitmap.memory = (u8*)malloc(data_size);
  memcpy(bitmap.memory, data, data_size);
  stbi_image_free(data);

  if (bitmap.memory == 0) 
    log_error("load_bitmap() could not load bitmap %s\n", file.path);

  bitmap.pitch = bitmap.width * bitmap.channels;
  //bitmap.mip_levels = (u32)floor(log2f((float32)max(bitmap.width, bitmap.height))) + 1;
  bitmap.mip_levels = 1;

  return bitmap;
}

internal void
save_bitmap(u32 id, FILE *file) {
  Bitmap *bitmap = find_bitmap(id);
  fwrite(bitmap, sizeof(Bitmap), 1, file);
  u32 size = bitmap->width * bitmap->height * bitmap->channels;
  fwrite(bitmap->memory, size, 1, file);
}

internal void
load_saved_bitmap(u32 id, FILE *file) {
  Bitmap *bitmap = find_bitmap(id);
  fread(bitmap, sizeof(Bitmap), 1, file);
  u32 size = bitmap->width * bitmap->height * bitmap->channels;
  fread(bitmap->memory, size, 1, file);
}

internal Bitmap
bitmap_resized(Bitmap *bitmap, Vector2_s32 dim) {
  Bitmap resized = {};
  u32 size = dim.width * dim.height * bitmap->channels;
  resized.memory = (u8*)malloc(size);
  stbir_resize_uint8(bitmap->memory, bitmap->width, bitmap->height, 0, resized.memory, dim.width, dim.height, 0, bitmap->channels);

  resized.channels = bitmap->channels;
  resized.dim = dim;
  resized.pitch = resized.width * resized.channels;
  resized.mip_levels = 1;

  return resized;
}

internal Bitmap
load_bitmap_flip(File file) {
  stbi_set_flip_vertically_on_load(true);
  Bitmap bitmap = load_bitmap(file);
  stbi_set_flip_vertically_on_load(false);
  return bitmap;
}

internal void
load_bitmap(u32 id, const char *filename) {
  String filepath = String(asset_folders[AT_BITMAP], filename);
  Bitmap *bitmap = find_bitmap(id);

  File file = load_file(filepath.str());
  *bitmap = load_bitmap(file);

  filepath.destroy();
}

internal void
bitmap_convert_channels(Bitmap *bitmap, u32 new_channels) {    
  if (new_channels != 1 && new_channels != 3 && new_channels != 4) {
    log_error("bitmap_convert_channels() not valid conversion (channels %d)\n", new_channels);
    return;
  }

  u8 *new_memory = (u8*)malloc(bitmap->width * bitmap->height * new_channels);

  for (s32 i = 0; i < bitmap->width * bitmap->height; i++) {
    u8 *src = bitmap->memory + (i * bitmap->channels);
    u8 *dest = new_memory + (i * new_channels);

    Color_RGBA color = {};
    switch(bitmap->channels) {
      case 1: color = {   0x00,   0x00,   0x00, src[0]}; break;
      case 3: color = { src[0], src[1], src[2], 0xFF  }; break;
      case 4: color = { src[0], src[1], src[2], src[3]}; break;
    }

    if (new_channels == 1) {
      dest[0] = color.a;
    } else if (new_channels == 3) {
      dest[0] = color.r;
      dest[1] = color.g;
      dest[2] = color.b;
    } else if (new_channels == 4) {
      dest[0] = color.r;
      dest[1] = color.g;
      dest[2] = color.b;
      dest[3] = color.a;
    }
  }

  free(bitmap->memory);
  bitmap->memory = new_memory;
  bitmap->channels = new_channels;
  bitmap->pitch = bitmap->width * bitmap->channels;
}

internal Bitmap
blank_bitmap(u32 width, u32 height, u32 channels) {
    Bitmap bitmap = {};
    bitmap.width = width;
    bitmap.height = height;
    bitmap.channels = channels;
    bitmap.pitch = bitmap.width * bitmap.channels;
    
    u32 bitmap_size = bitmap.width * bitmap.height * bitmap.channels;
    bitmap.memory = (u8 *)malloc(bitmap_size);
    memset(bitmap.memory, 0, bitmap_size);

    return bitmap;
}

internal u8
blend_alpha(u8 a1, u8 a2) {
    return 255 - ((255 - a1) * (255 - a2) / 255);
}

internal u8
blend_color(u8 c1, u8 c2, u8 alpha) {
    return (c1 * (255 - alpha) + c2 * alpha) / 255;
}

internal void
bitmap_copy_text(Bitmap dest, const Bitmap src, Vector2_s32 position, Color_RGB color) {
    if (src.width == 0 || src.height == 0)
        return;
    
    if (dest.channels != 4 || src.channels != 1) {
        log_error("bitmap_copy_text()", "not valid channels, valid means dest = 4 and src = 1 (dest = %d, src = %d)\n", dest.channels, src.channels);
        ASSERT(0);
    }
    
    if (dest.width < src.width || dest.height < src.height) {
        log_error("bitmap_copy_text() src bitmap is too big\n");
        ASSERT(0);
    }

    u8 *src_ptr = src.memory;
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *dest_ptr_line_start = dest_ptr;
    
    while(src_ptr < src.memory + (src.width * src.height * src.channels)) {
        u8 alpha = src_ptr[0];
        dest_ptr[0] = blend_color(dest_ptr[0], color.r, alpha);
        dest_ptr[1] = blend_color(dest_ptr[1], color.g, alpha);
        dest_ptr[2] = blend_color(dest_ptr[2], color.b, alpha);
        dest_ptr[3] = blend_alpha(dest_ptr[3], alpha);
            
        src_ptr += src.channels;
        dest_ptr += dest.channels;

        if (dest_ptr >= dest_ptr_line_start + (src.width * dest.channels)) {
            s32 src_line = u32(src_ptr - src.memory) / src.pitch;
            dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (src_line * dest.pitch);
            dest_ptr_line_start = dest_ptr;
        }
    }
}

// color contains what to fill conversion from 1 channel to 4 channels with
internal void
copy_blend_bitmap(Bitmap dest, const Bitmap src, Vector2_s32 position) {
    if (src.width == 0 || src.height == 0)
        return;

    if (dest.channels != src.channels) {
        log_error("copy_blend_bitmap() bitmap channels do not match (%d != %d)\n", dest.channels, src.channels);
        ASSERT(0);
    }

    if (dest.width < src.width || dest.height < src.height) {
        log_error("copy_blend_bitmap() src bitmap is too big\n");
        ASSERT(0);
    }
    
    u8 *src_ptr = src.memory;
    u8 *dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch);
    u8 *dest_ptr_line_start = dest_ptr;
    
    while(src_ptr < src.memory + (src.width * src.height * src.channels)) {
        
        for (s32 channel_index = 0; channel_index < dest.channels; channel_index++) {
            
            if (dest.channels == 1 || (dest.channels == 4 && channel_index == 3)) {
                dest_ptr[channel_index] = blend_alpha(dest_ptr[channel_index], src_ptr[channel_index]);
            } else if (dest.channels == 3) {
                dest_ptr[channel_index] = blend_color(dest_ptr[channel_index], src_ptr[channel_index], 0xFF);
            } else if (dest.channels == 4) {
                dest_ptr[channel_index] = blend_color(dest_ptr[channel_index], src_ptr[channel_index], src_ptr[3]);
            }        
        }

        src_ptr += src.channels;
        dest_ptr += dest.channels;

        // Go to next line in dest bitmap
        if (dest_ptr >= dest_ptr_line_start + src.pitch) {
            s32 src_line = u32(src_ptr - src.memory) / src.pitch;
            dest_ptr = dest.memory + (position.x * dest.channels) + (position.y * dest.pitch) + (src_line * dest.pitch);
            dest_ptr_line_start = dest_ptr;

            // going to write beyond dest
            if (position.y + src_line >= dest.height)
                return;
        }
    }
}

internal void
write_bitmap(Bitmap *bitmap, const char *file_path) {
  stbi_write_png(file_path, bitmap->width, bitmap->height, bitmap->channels, bitmap->memory, bitmap->pitch);
}

/*
  Texture Atlas
*/

// create a blank texture atlas
internal Texture_Atlas
create_texture_atlas(u32 width, u32 height, u32 channels) {
    Texture_Atlas atlas = {};
    atlas.bitmap = blank_bitmap(width, height, channels);

    return atlas;
}

internal void
texture_atlas_reset(Texture_Atlas *atlas) {
    u32 bitmap_size = atlas->bitmap.width * atlas->bitmap.height * atlas->bitmap.channels;
    memset(atlas->bitmap.memory, 0, bitmap_size);

    atlas->texture_count = 0;
    atlas->insert_position = { 0, 0 };
    atlas->row_height = 0;

    atlas->resetted = true;
}

internal u32
texture_atlas_add(Texture_Atlas *atlas, Bitmap *bitmap) {
  Vector2_s32 padding = { 1, 1 };
  
  Vector2_s32 position = {};

  // Check if bitmap fits next in line
  if (atlas->insert_position.x + bitmap->width  + padding.x < atlas->bitmap.width &&
    atlas->insert_position.y + bitmap->height + padding.y < atlas->bitmap.height) {
    position = atlas->insert_position;
  // Check if the bitmap fits on the next line
  } else if (atlas->insert_position.y + atlas->row_height + bitmap->height + padding.y < atlas->bitmap.height) {
    position = { 0, atlas->insert_position.y + atlas->row_height + padding.y };
    atlas->row_height = 0;
  // No more space reset atlas
  } else {
    texture_atlas_reset(atlas);
    log_error("texture_atlas_add() not enough room for bitmap\n");
  }

  copy_blend_bitmap(atlas->bitmap, *bitmap, position);

  Vector2 tex_coords_p1 = { 
    (float32)position.x / (float32)atlas->bitmap.width, 
    (float32)position.y / (float32)atlas->bitmap.height 
  };
  Vector2 tex_coords_p2 = { 
    float32(position.x + bitmap->width)  / (float32)atlas->bitmap.width, 
    float32(position.y + bitmap->height) / (float32)atlas->bitmap.height 
  };

  if (tex_coords_p1.x < EPSILON)
    tex_coords_p1.x = 0.0f;
  if (tex_coords_p1.y < EPSILON)
    tex_coords_p1.y = 0.0f;
  if (tex_coords_p2.x < EPSILON)
    tex_coords_p2.x = 0.0f;
  if (tex_coords_p2.y < EPSILON)
    tex_coords_p2.y = 0.0f;
  
  atlas->texture_coords[atlas->texture_count].p1 = tex_coords_p1;
  atlas->texture_coords[atlas->texture_count].p2 = tex_coords_p2;
  
  u32 index = atlas->texture_count++;

  if (bitmap->height > atlas->row_height) {
      atlas->row_height = bitmap->height;
  }
  atlas->insert_position = { position.x + padding.x + bitmap->width, position.y };

  for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      atlas->gpu[i].refresh_required = true;
  }

  return index;
}

internal void
texture_atlas_add(Texture_Atlas *atlas, const char *file_path) {
    File file = load_file(file_path);
    Bitmap bitmap = load_bitmap(file);
    texture_atlas_add(atlas, &bitmap);
}

internal void
texture_atlas_init_gpu_frame(Texture_Atlas *atlas, u32 frame_index) {
  vulkan_destroy_texture(atlas->gpu[frame_index].handle);
  atlas->gpu[frame_index].handle = vulkan_create_texture(&atlas->bitmap, TEXTURE_PARAMETERS_CHAR);
  Descriptor texture_desc = gfx_descriptor(&atlas->gpu[frame_index].set, 1);
  vulkan_set_texture(&texture_desc, atlas->gpu[frame_index].handle);
}

internal void
texture_atlas_refresh(Texture_Atlas *atlas) {
    u32 current_frame = vk_ctx.current_frame;
    Texture_Atlas_GPU *gpu = &atlas->gpu[current_frame];
    gpu->set.texture_index = 0;
    if (gpu->refresh_required) {
        gpu->refresh_required = false;
        texture_atlas_init_gpu_frame(atlas, current_frame);
    }    
}

internal void
texture_atlas_init_gpu(Texture_Atlas *atlas) {
  for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    texture_atlas_init_gpu_frame(atlas, i);
  }
}

internal void
texture_atlas_init(Texture_Atlas *atlas, u32 layout_id) {
    atlas->gpu[0].set = gfx_descriptor_set(layout_id);
    atlas->gpu[1].set = gfx_descriptor_set(layout_id);

    texture_atlas_init_gpu(atlas);
}

// @Warning shader and desc binded before
internal void
texture_atlas_draw_rect(Texture_Atlas *atlas, u32 index, Vector2 coords, Vector2 dim) {
  Texture_Coords tex_coord = atlas->texture_coords[index];
  
  vulkan_immediate_vertex_xu(Vertex_XU{ coords, tex_coord.p1 });
  vulkan_immediate_vertex_xu(Vertex_XU{ coords + dim, tex_coord.p2 });
  vulkan_immediate_vertex_xu(Vertex_XU{ { coords.x, coords.y + dim.y }, {tex_coord.p1.x, tex_coord.p2.y} });
  
  vulkan_immediate_vertex_xu(Vertex_XU{ coords, tex_coord.p1 });
  vulkan_immediate_vertex_xu(Vertex_XU{ { coords.x + dim.x, coords.y }, {tex_coord.p2.x, tex_coord.p1.y} });
  vulkan_immediate_vertex_xu(Vertex_XU{ coords + dim, tex_coord.p2 });
  
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {{2,0,0,0}, {0,0,0,0}};
  vulkan_update_ubo(color_desc, (void *)&local);
  Descriptor texture_desc = gfx_descriptor(&local_desc_set, 1);
  vulkan_set_bitmap(&texture_desc, &atlas->bitmap);
  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = identity_m4x4();
  vulkan_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

  vulkan_draw_immediate(6);
}

internal void
texture_atlas_write(Texture_Atlas *atlas, const char *bitmap_file_name, const char *tex_coord_file_name) {    
  String bitmap_filepath = String(asset_folders[AT_ATLAS], bitmap_file_name);
  String texture_coords_filepath = String(asset_folders[AT_ATLAS], tex_coord_file_name);

  write_bitmap(&atlas->bitmap, bitmap_filepath.str());
  
  FILE *file;
  fopen_s(&file, texture_coords_filepath.str(), "wb");
  if (file) {
    fwrite(atlas->texture_coords, sizeof(Texture_Coords) * atlas->max_textures, 1, file);
    fclose(file);
  } else {
    log_error("texture_atlas_write() Could not open file %s\n", texture_coords_filepath.str());
  }

  bitmap_filepath.destroy();
  texture_coords_filepath.destroy();
}
internal void
texture_atlas_draw_rect(u32 id, u32 index, Vector2 coords, Vector2 dim) {
  Texture_Atlas *atlas = find_atlas(id);
  texture_atlas_draw_rect(atlas, index, coords, dim);
}

internal void
texture_atlas_destroy(Texture_Atlas *atlas) {
  for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vulkan_destroy_texture(atlas->gpu[i].handle);
  }
}

internal void
load_atlas(u32 id, const char *image_filename, const char *tex_coords_filename) {
  Texture_Atlas *atlas = find_atlas(id);
  String bitmap_filepath = String(asset_folders[AT_ATLAS], image_filename);
  String tex_coords_filepath = String(asset_folders[AT_ATLAS], tex_coords_filename);

  File bitmap_file = load_file(bitmap_filepath.str());
  File tex_coords_file = load_file(tex_coords_filepath.str());
  atlas->bitmap = load_bitmap(bitmap_file);
  memcpy(atlas->texture_coords, tex_coords_file.memory, sizeof(Texture_Coords) * atlas->max_textures);

  bitmap_filepath.destroy();
  tex_coords_filepath.destroy();
}

internal Texture_Region
atlas_region(Texture_Atlas *atlas, u32 index) {
  Texture_Coords *uv = &atlas->texture_coords[index];
  float32 width = uv->p2.x - uv->p1.x;
  float32 height = uv->p2.y - uv->p1.y;
  Texture_Region region = {};
  region.uv_offset = { uv->p1.x, uv->p1.y };
  region.uv_scale = { width, height };
  return region;
}

/*
  Font
*/

internal void
load_font(u32 id, const char *filename) {
  String filepath = String(asset_folders[AT_FONT], filename);
  File file = load_file(filepath.str());
  filepath.destroy();

  Font *font = find_font(id);

  font->info = malloc(sizeof(stbtt_fontinfo));
  stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
  *info = {};

  font->cache = (Font_Cache *)malloc(sizeof(Font_Cache));
  memset(font->cache, 0, sizeof(Font_Cache));
  memset(font->cache->bitmaps, 0, sizeof(Font_Char_Bitmap) * ARRAY_COUNT(font->cache->bitmaps));

  stbtt_InitFont(info, (u8*)file.memory, stbtt_GetFontOffsetForIndex((u8*)file.memory, 0));
  stbtt_GetFontBoundingBox(info, &font->bb_0.x, &font->bb_0.y, &font->bb_1.x, &font->bb_1.y);

  font->cache->atlas = create_texture_atlas(1000, 1000, 1);
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

  memset(font_char, 0, sizeof(Font_Char));
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
      log_error("load_font_char_bitmap() scale can not be zero"); // scale used below
      return 0;
  }

  Texture_Atlas *atlas = &font->cache->atlas;
  stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

  if (atlas->resetted) {
      print("HERE\n");
      atlas->resetted = false;
      font->cache->bitmaps_cached = 0;
  }

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

  memset(char_bitmap, 0, sizeof(Font_Char_Bitmap));
  char_bitmap->font_char = load_font_char(font, codepoint);
  char_bitmap->scale = scale;
  char_bitmap->bitmap.memory = stbtt_GetGlyphBitmapSubpixel(info, 0, char_bitmap->scale, 0, 0, char_bitmap->font_char->glyph_index, &char_bitmap->bitmap.width, &char_bitmap->bitmap.height, 0, 0);
  char_bitmap->bitmap.channels = 1;
  char_bitmap->bitmap.pitch = char_bitmap->bitmap.width * char_bitmap->bitmap.channels;

  stbtt_GetGlyphBitmapBox(info, char_bitmap->font_char->glyph_index, char_bitmap->scale, char_bitmap->scale, &char_bitmap->bb_0.x, &char_bitmap->bb_0.y, &char_bitmap->bb_1.x, &char_bitmap->bb_1.y);

  char_bitmap->index = texture_atlas_add(atlas, &char_bitmap->bitmap);
  //char_bitmap->bitmap.mip_levels = 1;
  //vulkan_create_texture(&char_bitmap->bitmap, TEXTURE_PARAMETERS_CHAR);

  return char_bitmap;
}

internal void
clear_font_bitmap_cache(Font *font) {
  stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
  for (s32 i = 0; i < font->cache->bitmaps_cached; i++) {
      stbtt_FreeBitmap(font->cache->bitmaps[i].bitmap.memory, info->userdata);
      font->cache->bitmaps[i].bitmap.memory = 0;
      vulkan_destroy_texture(&font->cache->bitmaps[i].bitmap);
      memset(&font->cache->bitmaps[i], 0, sizeof(Font_Char_Bitmap));
  }

  font->cache->bitmaps_cached = 0;
}

float32 get_scale_for_pixel_height(Font *font, float32 pixel_height) {
    return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)font->info, pixel_height);
}

float32 get_scale_for_pixel_height(u32 font_id, float32 pixel_height) {
  Font *font = find_font(font_id);
  return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)font->info, pixel_height);
}

s32 get_codepoint_kern_advance(void *info, s32 ch1, s32 ch2) {
    return stbtt_GetCodepointKernAdvance((stbtt_fontinfo*)info, ch1, ch2);
}

s32 get_glyph_kern_advance(void *info, s32 gl1, s32 gl2) {
    return stbtt_GetGlyphKernAdvance((stbtt_fontinfo*)info, gl1, gl2);
}

/*
  Assets
*/

internal s32
load_assets_switch(u32 i, u32 asset_type, Asset_Load *loads) {
  switch(asset_type) {
    case AT_SHADER: {
      u32 filenames_count = ARRAY_COUNT(loads[0].filenames);
      Pipeline *pipeline = find_pipeline(pipeline_loads[i].id);
      Asset_Load *load = &pipeline_loads[i];

      for (u32 file_index = 0; file_index < filenames_count; file_index++) {
        if (!load->filenames[file_index]) {
          continue;
        }

        s32 result = load_shader_file(pipeline, load->filenames[file_index]);
        if (result == FAILURE) {
          return FAILURE;
        }
      }

      spirv_compile_shader(pipeline);
    } break;
    case AT_BITMAP:
      load_bitmap(loads[i].id, loads[i].filenames[0]);
      break;
    case AT_FONT:
      load_font(loads[i].id, loads[i].filenames[0]);
      break;
    case AT_ATLAS:
      load_atlas(loads[i].id, loads[i].filenames[0], loads[i].filenames[1]);
      break;
    case AT_GEOMETRY:
      load_geometry(loads[i].id, loads[i].filenames[0]);
      break;
  }

  return SUCCESS;
}

internal s32
save_assets_switch(u32 i, u32 asset_type, FILE *file) {
  switch(asset_type) {
    case AT_SHADER:
      save_pipeline(i, file);
      break;
    case AT_BITMAP:
      save_bitmap(i, file);
      break;
    case AT_FONT:
      break;
    case AT_ATLAS:
      break;
    case AT_GEOMETRY:
      break;
  }

  return SUCCESS;
}

internal s32
load_saved_assets_switch(u32 i, u32 asset_type, FILE *file) {
  switch(asset_type) {
    case AT_SHADER:
      load_saved_pipeline(i, file);
      break;
    case AT_BITMAP:
      load_saved_bitmap(i, file);
      break;
    case AT_FONT:
      break;
    case AT_ATLAS:
      break;
    case AT_GEOMETRY:
      break;
  }

  return SUCCESS;
}

internal s32 
load_assets(Asset_Array *asset_array, Asset_Load *loads, u32 loads_count, u32 asset_type) {
  print("loading assets (%s)...\n", asset_folders[asset_type]);

  u32 filenames_count = ARRAY_COUNT(loads[0].filenames);

  prepare_asset_array(asset_array, loads_count, asset_size[asset_type]);   

  if (asset_type == AT_BITMAP && asset_type == AT_SHADER)
    loads_count = 0;

  for (u32 i = 0; i < loads_count; i++) {
    if (debug.load_assets) {
      load_assets_switch(i, asset_type, loads);
      save_assets_switch(i, asset_type, debug.asset_save_file);
    } else {
      load_saved_assets_switch(i, asset_type, debug.asset_load_file);
    }
  }

  // post load
  if (asset_type == AT_SHADER) {
    gfx_add_layouts_to_shaders();
  }

  return SUCCESS;
}

internal s32 
init_assets() {
  init_pipelines();

  for (u32 i = 0; i < assets.bitmaps.count; i++) {
    Bitmap *bitmap = find_bitmap(i);
    vulkan_create_texture(bitmap, TEXTURE_PARAMETERS_DEFAULT);
  }

  for (u32 i = 0; i < assets.fonts.count; i++) {
    Font *font = find_font(i);
    texture_atlas_init(&font->cache->atlas, GFXID_TEXTURE);
  }

  for (u32 i = 0; i < assets.atlases.count; i++) {
    Texture_Atlas *atlas = find_atlas(i);
    texture_atlas_init(atlas, GFXID_TEXTURE);
  }

  for (u32 i = 0; i < assets.geometrys.count; i++) {
    Geometry *geo = find_geometry(i);
    init_geometry(geo);
  }

  return SUCCESS;
}

internal void
assets_cleanup() {
  for (u32 i = 0; i < assets.pipelines.count; i++) {
    Pipeline *pipeline = find_pipeline(i);
    vulkan_pipeline_cleanup(pipeline);
  }

  for (u32 i = 0; i < assets.bitmaps.count; i++) {
    Bitmap *bitmap = find_bitmap(i);
    vulkan_destroy_texture(bitmap);
  }

  for (u32 i = 0; i < assets.fonts.count; i++) {
    Font *font = find_font(i);
    texture_atlas_destroy(&font->cache->atlas);
  }

  for (u32 i = 0; i < assets.atlases.count; i++) {
    Texture_Atlas *atlas = find_atlas(i);
    texture_atlas_destroy(atlas);
  }

  for (u32 i = 0; i < assets.geometrys.count; i++) {
    Geometry *geo = find_geometry(i);
    destroy_geometry(geo);
  }
}

struct Load_Assets_Args {
  Asset_Array *asset_array;
  Asset_Load *loads;
  u32 loads_count;
  u32 asset_type;
};

internal s32
load_assets_thread(void *ptr) {
  Load_Assets_Args *args = (Load_Assets_Args *)ptr;
  return load_assets(args->asset_array, args->loads, args->loads_count, args->asset_type);
}
