/*
  File
*/

File load_file(const char *filepath) {
  File result = {};
    
  //FILE *in = fopen(filepath, "rb");
  FILE *in;
  fopen_s(&in, filepath, "rb");
  if(in) {
    fseek(in, 0, SEEK_END);
    result.size = ftell(in);
    fseek(in, 0, SEEK_SET);
        
    result.memory = malloc(result.size);
    fread(result.memory, result.size, 1, in);
    fclose(in);
  } else { 
    log_error("load_file(): Cannot open file %s\n", filepath);
  }
        
  return result;
}

void destroy_file(File *file) {
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
    log_error("compile_glsl_to_spirv(): %s\n", error_message);
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

u32 identify_shader_stage(const char *filename) {
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

s32 load_shader_file(Shader *shader, const char *filename) {
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

s32 load_pipelines() {
  print("loading pipelines...\n");

  u32 filenames_count = ARRAY_COUNT(pipeline_loads[0].filenames);
  u32 pipeline_loads_count = ARRAY_COUNT(pipeline_loads);

  prepare_asset_array(&assets.pipelines, pipeline_loads_count, sizeof(Pipeline));

  for (u32 i = 0; i < pipeline_loads_count; i++) {
    Pipeline *pipeline = find_pipeline(pipeline_loads[i].id);
    Pipeline_Load *load = &pipeline_loads[i];

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
  }

  gfx.init(); // set up descriptor set layouts

  return SUCCESS;
}

s32 init_pipelines() {
  print("initializing pipelines...\n");
  u32 pipeline_loads_count = ARRAY_COUNT(pipeline_loads);
  for (u32 i = 0; i < pipeline_loads_count; i++) {
    Pipeline *pipeline = find_pipeline(pipeline_loads[i].id);
    gfx.create_graphics_pipeline(pipeline, gfx.draw_render_pass);
  }

  return SUCCESS;
}