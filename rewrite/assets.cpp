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

inline u32 get_length(const char *string) {
  if (string == 0)
    return 0;
    
  u32 length = 0;
  const char *ptr = string;
  while(*ptr != 0)
  {
    length++;
    ptr++;
  }
  return length;
}

const char* get_filename(const char *filepath) {
  if (filepath == 0)
    return 0;

  u32 length = get_length(filepath);
  char *ptr = (char*)filepath;
  ptr += length;

  u32 name_length = 0;
  while(*ptr != '/') {
    ptr--;
    name_length++;
  }
  ptr++;
  name_length--;

  char *filename = (char*)malloc(name_length + 1);
  for (u32 i = 0; i < name_length; i++) {
    filename[i] = ptr[i];
  }
  filename[name_length] = 0;

  return filename;
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

/*
File compile_glsl_to_spirv(File *file, shaderc_compiler_t compiler, u32 shader_kind, shaderc_compile_options_t options) {
  const char *filename = get_filename(file->filepath);
  const shaderc_compilation_result_t result = shaderc_compile_into_spv(compiler, (char*)file->memory, file->size, (shaderc_shader_kind)shader_kind, filename, "main", options);
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

  return result_file;
}

void shader_classify_files(Shader *shader) {
  
}

void spirv_compile_shader(Shader *shader) {
  shaderc_compiler_t compiler = shaderc_compiler_initialize();
  shaderc_compile_options_t options = shaderc_compile_options_initialize();

  for (u32 i = 0; i < SHADER_STAGES_COUNT; i++) {
    if (shader->glsl_files[i].memory == 0) 
      continue; // file was not loaded
    
    shader->spirv_files[i] = compile_glsl_to_spirv(&shader->glsl_files[i], compiler, shaderc_glsl_file_types[i], options);
    if (shader->spirv_files[i].size == 0) {
      printf("compile_shader() could not compile %s\n", shader->glsl_files[i].filepath); 
      return;
    }
  }

  shaderc_compile_options_release(options);
  shaderc_compiler_release(compiler);
}
*/
//
// Asset Manager
//

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

  // count how many assets there are, not how many files
  std::vector<Asset_Load_Info> unique_load_infos;
  for (const Asset_Load_Info& load : assets->load_info) {
    bool found = false;
    for (const Asset_Load_Info& unique_load : unique_load_infos) {
      if (load.type == unique_load.type && load.id == unique_load.id) {
        found = true;
        break;
      }
    }

    if (!found) {
      unique_load_infos.push_back(load);
    } else {
      assets->arrays[load.type].count--;
    }
  }

  // allocate asset arrays
  for (u32 type_index = 0; type_index < ASSET_TYPE_COUNT; type_index++) {
    Asset_Array *array = &assets->arrays[type_index];
    array->type = type_index;
    array->size = array->count * asset_type_sizes[type_index];
    array->memory = malloc(array->size);
  }

  s32 file_count = (s32)assets->load_info.size();
  printf("Num of asset files: %d\n", file_count);

  assets->files = ARRAY_MALLOC(File, file_count);

  load_asset_files(assets);

  return 0;
}