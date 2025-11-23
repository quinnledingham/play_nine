internal void
load_font(u32 id, const char *filename) {
  Font *font = find_font(id);
  String filepath = String(asset_folders[AT_FONT], filename);

  //File file = load_file(filepath.str());
  font->ttf_font = TTF_OpenFont(filepath.str(), 18.0f);

  if (!font->ttf_font) {
    app_log("failed to load font (@ %s)\n", filepath.str());
  }

  filepath.destroy();
}

//
// Loading Assets
//

internal void
prepare_asset_array(Assets_Load_Info *info) {
  Asset_Array *arr = info->asset_array;
  if (arr->count == 0) {
    arr->count = info->loads_count;
    arr->type_size = asset_sizes[info->asset_type];
    arr->buffer = blank_buffer(arr->count * arr->type_size);
  } else {
    arr->buffer.clear();
  }
}

internal s32
load_assets_switch(Asset_Load *load, u32 asset_type) {
  switch(asset_type) {
    /*
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
    */
    case AT_FONT:
      load_font(load->id, load->filenames[0]);
      break;
    default:
      app_log_error("load_assets_switch(): ASSET NOT IMPLEMENTED (folder: %s)", asset_folders[asset_type]);
      return FAILURE;
      break;
  }

  return SUCCESS;
}

internal s32
load_assets(Assets_Load_Info *info) {
  app_log("loading assets (%s)...\n", asset_folders[info->asset_type]);

  prepare_asset_array(info);

  for (u32 i = 0; i < info->loads_count; i++) {
    if (load_assets_switch(&info->loads[i], info->asset_type) == FAILURE) {
      return 1;
    }
  }

  // post load
  //if (info.asset_type == AT_SHADER) {
  //  gfx_add_layouts_to_shaders();
  //}

  return 0;
}
