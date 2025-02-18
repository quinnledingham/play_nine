//
// File
//

File load_file(const char *filepath) {
  File result = {};
    
  FILE *in = fopen(filepath, "rb");
  if(in) {
    fseek(in, 0, SEEK_END);
    result.size = ftell(in);
    fseek(in, 0, SEEK_SET);
        
    result.memory = malloc(result.size);
    fread(result.memory, result.size, 1, in);
    fclose(in);
  } else { 
    printf("load_file(): Cannot open file %s\n", filepath);
  }
        
  return result;
}

//
// Shader
//

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
    printf("compile_glsl_to_spirv(): %s\n", error_message);
  }

  u32 length = (u32)shaderc_result_get_length(result);
  const char *bytes = shaderc_result_get_bytes(result);

  File result_file = {};
  result_file.memory = malloc(length);
  result_file.size = length;
  memset(result_file.memory, 0, length);
  memcpy(result_file.memory, (void *)bytes, length);

  shaderc_result_release(result);

  file->spirv = result_file;

  if (file->spirv.size == 0) {
    printf("compile_glsl_to_spirv() could not compile %s\n", file->filename); 
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
    if (file->filename == 0) 
      continue; // file was not loaded
    
    compile_glsl_to_spirv(file, compiler, shaderc_glsl_file_types[i], options);
    spirv_reflection(file);
  }

  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);
}

// identifies what the shader stage is based on the file extension
u32 identify_shader_stage(const char *filename) {
  // find end of filename
  char *ptr = (char *)filename;
  while (*ptr != 0) {
    ptr++;
  }

  // from end find first period
  while (*ptr != '.' && ptr > filename) {
    ptr--;
  }
  
  // compare it to various shader file type endings
  for (u32 stage_index = 0; stage_index < SHADER_STAGES_COUNT; stage_index++) {
    const char *stage_file = shader_file_types[stage_index];
    if (!strcmp(ptr, stage_file))
      return stage_index; // corresponds to a shader stage enum
  }

  printf("(assets) identify_shader_stage(): DID NOT FIND SHADER FILE TYPE (%s)\n", filename);
  return SHADER_STAGES_COUNT;
}

//
// Asset Manager
//

s32 allocate_assets(u32 type, u32 count) {
  Asset_Array *array = &app.assets.arrays[type];
  array->memory = malloc(count * asset_type_sizes[type]);
  return 0;
}

s32 add_asset(u32 type, u32 id, const char *filename) {
  Asset_Load_Info load = {};
  load.type = type;
  load.id = id;
  load.filename = filename;
  load.filename = get_asset_folder(type) + load.filename; // add asset type folder location
  app.assets.load_info.push_back(load);

  app.assets.arrays[type].count++;

  return 0;
}

s32 load_asset_files(Assets *assets) {

  u32 files_index = 0;
  for (const Asset_Load_Info& load : assets->load_info) {
    assets->files[files_index++] = load_file(load.filename.c_str());
  }

  return 0;
}

s32 init_assets(Assets *assets) {
  for (u32 type_index = 0; type_index < ASSET_TYPE_COUNT; type_index++) {
    Asset_Array *array = &assets->arrays[type_index];
    switch (type_index) {
      case ASSET_TYPE_SHADER: {
        for (u32 shader_index = 0; shader_index < array->count; shader_index++) {
          Shader *shader = ((Shader *)array->memory) + shader_index;
        }
      } break;
    }
  }
  return 0;
}

s32 load_assets(Assets *assets) {

  s32 file_count = (s32)assets->load_info.size();
  printf("(assets) Num of asset files: %d\n", file_count);

  assets->files = ARRAY_MALLOC(File, file_count);

  load_asset_files(assets);

  // This loads the files into the asset arrays.
  // Assets in the asset arrays should be ready to use or to be sent to the gpu after this.
  // Works becuase asset load info aligns with the file array.
  u32 file_index = 0;
  for (const Asset_Load_Info& load : assets->load_info) {
    File *file = &assets->files[file_index++];
    Asset_Array *array = &assets->arrays[load.type];
    switch (load.type) {
      case ASSET_TYPE_SHADER: {
        u32 shader_stage = identify_shader_stage(load.filename.c_str());
        Shader *shader = (Shader *)array->memory + load.id;

        Shader_File *shader_file = &shader->files[shader_stage];
        shader_file->glsl = *file;
        shader_file->filename = load.filename.c_str();
        shader_file->stage = shader_stage;
      } break;

      case ASSET_TYPE_BITMAP: {

      } break;
    }
  }

  return 0;

}