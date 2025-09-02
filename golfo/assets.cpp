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
    app_log_error("load_file(): Cannot open file %s\n", filepath);
  }

  // Save filepath
  result.path = String(filepath);

  return result;
}

internal void 
destroy_file(File *file) {
  if (!file->memory)
    return;

  SDL_free(file->memory);
  file->memory = 0;
  file->size = 0;
}

File buffer_to_file(Buffer buffer) {
  File result = {};
  result.memory = buffer.memory;
  result.size = buffer.size;
  return result;
}

//
// Texture
//

internal Texture
load_texture(File file) {
  Texture bitmap = {};
  bitmap.channels = 4;

  // 4 arg always get filled in with the original amount of channels the image had.
  // Currently forcing it to have 4 channels.
  s32 width;
  s32 height;
  unsigned char *data = stbi_load_from_memory((stbi_uc const *)file.memory, file.size, &width, &height, &bitmap.channels, 0);
  bitmap.width = (u32)width;
  bitmap.height = (u32)height;
  u32 data_size = bitmap.width * bitmap.height * bitmap.channels;
  bitmap.data = (u8*)malloc(data_size);
  memcpy(bitmap.data, data, data_size);
  //stbi_image_free(data);

  if (bitmap.data == 0) 
    app_log_error("load_bitmap() could not load bitmap %s\n", file.path);

  bitmap.pitch = bitmap.width * bitmap.channels;
  //bitmap.mip_levels = (u32)floor(log2f((float32)max(bitmap.width, bitmap.height))) + 1;
  bitmap.mip_levels = 1;

  return bitmap;
}

internal void
load_texture(u32 id, const char *filename) {
  String filepath = String(asset_folders[AT_TEXTURE], filename);
  Texture *texture = find_texture(id);

  File file = load_file(filepath.str());
  *texture = load_texture(file);

  filepath.destroy();
}

internal void
texture_convert_channels(Texture *bitmap, u32 new_channels) {    
  if (new_channels != 1 && new_channels != 3 && new_channels != 4) {
    app_log_error("bitmap_convert_channels() not valid conversion (channels %d)\n", new_channels);
    return;
  }

  u8 *new_memory = (u8*)malloc(bitmap->width * bitmap->height * new_channels);

  for (u32 i = 0; i < bitmap->width * bitmap->height; i++) {
    u8 *src = bitmap->data + (i * bitmap->channels);
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

  free(bitmap->data);
  bitmap->data = new_memory;
  bitmap->channels = new_channels;
  bitmap->pitch = bitmap->width * bitmap->channels;
}

/*
  Shader
*/

/*
  shaderc_compile_options_set_target_env(options, shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
*/

internal void 
compile_glsl_to_spirv(Shader_File *file, shaderc_compiler_t compiler, u32 shader_kind, shaderc_compile_options_t options) {
  const char *filename = get_filename(file->filename);
  const shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, (char*)file->glsl.memory, file->glsl.size, (shaderc_shader_kind)shader_kind, filename, "main", options);
  free((void*)filename);

  u32 num_of_warnings = (u32)shaderc_result_get_num_warnings(result);
  u32 num_of_errors = (u32)shaderc_result_get_num_errors(result);

  if (num_of_warnings != 0 || num_of_errors != 0) {
    const char *error_message = shaderc_result_get_error_message(result);
    u32 error_size = str_length(error_message);
    app_log_error("(%s) compile_glsl_to_spirv(): %s\n", file->filename, error_message);
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
    app_log_error("compile_glsl_to_spirv() could not compile %s\n", file->filename); 
    return;
  }
}

internal void 
spirv_reflection(Shader_File *file) {
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

internal void 
spirv_compile_shader(Shader *shader) {
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  for (u32 i = 0; i < shader->files_count; i++) {
    Shader_File *file = &shader->files[i];
    if (!file->loaded) 
      continue; // file was not loaded
    
    compile_glsl_to_spirv(file, compiler, shaderc_glsl_file_types[file->stage], options);
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

  app_log_error("identify_shader_stage(): count not find a stage that matched -> %s, returning default\n", filename);

  return 0;
}

internal s32 
load_shader_file(Shader *shader, const char *filename) {
  u32 stage = identify_shader_stage(filename);

  if (stage == SHADER_STAGE_COMPUTE) {
    shader->compute = true;
  }

  if (shader->files_count == SHADER_STAGES_COUNT) {
    app_log_error("load_shader_file(): tried to add too many files/stages\n");
    return FAILURE;
  }

  Shader_File *file = &shader->files[shader->files_count++];

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
          app_log("loaded shader file: %s\n", filepath_ending.str());
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
init_pipelines() {
  app_log("initializing pipelines...\n");
  for (u32 i = 0; i < assets.pipelines.count; i++) {
    Pipeline *pipeline = find_pipeline(i);
    if (pipeline->compute) {
      //vulkan_create_compute_pipeline(pipeline);
    } else {
      vulkan_create_graphics_pipeline(&vk_ctx, pipeline, vk_ctx.draw_render_pass);
    } 
  }

  return SUCCESS;
}


/*
  Assets Loading
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
    case AT_TEXTURE:
      load_texture(loads[i].id, loads[i].filenames[0]);
      break;
  }

  return SUCCESS;
}

internal s32
load_assets(Assets_Load_Info info) {
  app_log("loading assets (%s)...\n", asset_folders[info.asset_type]);

  u32 filenames_count = ARRAY_COUNT(info.loads[0].filenames);
  prepare_asset_array(info.asset_array, info.loads_count, asset_size[info.asset_type]);   

  for (u32 i = 0; i < info.loads_count; i++) {
    load_assets_switch(i, info.asset_type, info.loads);
  }

  // post load
  if (info.asset_type == AT_SHADER) {
    gfx_add_layouts_to_shaders();
  }

  return 0;
}

internal s32 
init_assets() {
  init_pipelines();


  for (u32 i = 0; i < assets.textures.count; i++) {
    Texture *bitmap = find_texture(i);
    vulkan_create_texture(bitmap, TEXTURE_PARAMETERS_DEFAULT);
  }
/*
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
*/

  return SUCCESS;
} 