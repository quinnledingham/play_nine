internal void
init_types_array(Asset *data, Asset_Array *types) {
    u32 running_total_of_assets = 0;
    for (u32 i = 0; i < ASSET_TYPE_COUNT; i++) {
        types[i].data = data + (running_total_of_assets);
        running_total_of_assets += types[i].size;
    }
}


D3D12_Texture tex = {};

internal void
load_asset(Asset *asset, Asset_Load *load, u32 type) {
  *asset = {};
  asset->type = type;
  asset->tag = load->tag;
  switch(asset->type) {
    case ASSET_TYPE_BITMAP: {
      const char *filepath = str_concat(asset_folders[ASSET_TYPE_BITMAP], load->filename);
      asset->files[0] = load_file(filepath);
      asset->bitmap = load_bitmap(asset->files[0], false);
      d3d12_init_bitmap(&tex, &asset->bitmap);
    } break;
  }
}

internal void
load_assets(Assets *assets) {
  // count assets
  for (u32 type_index = 0; type_index < ASSET_TYPE_COUNT; type_index++) {
    if (assets->loads[type_index].data != 0) {
      assets->types[type_index].size = assets->loads[type_index].size;
      assets->count += assets->loads[type_index].size;
    }
  }

  // init assets arrays
  assets->data = ARRAY_MALLOC(Asset, assets->count);
  init_types_array(assets->data, assets->types);

  // load asset files

  for (u32 type_index = 0; type_index < ASSET_TYPE_COUNT; type_index++) {
    if (assets->loads[type_index].data == 0)
      continue;
    
    for (u32 asset_index = 0; asset_index < assets->loads[type_index].size; asset_index++) {
      load_asset((Asset *)assets->types[type_index].get(asset_index), (Asset_Load *)assets->loads[type_index].get(asset_index), type_index);
    }
  }

}
