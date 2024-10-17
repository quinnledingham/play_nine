internal void
load_asset(Asset *asset, File file) {
  switch(asset->type) {
    case ASSET_TYPE_BITMAP: {
      asset->bitmap = load_bitmap(file, false);
    } break;
  }
}

internal void
load_assets(Assets *assets) {
  u32 assets_count = 0;
  for (u32 type_index = 0; type_index < ASSET_TYPE_COUNT; type_index++) {
    if (assets->loads[type_index].data != 0) {
      assets_count += assets->loads[type_index].size;
    }
  }
  print("%d\n", assets_count);

  assets->files.data = iru_malloc(sizeof(Asset_Files) * assets_count);
  assets->files.element_size = sizeof(Asset_Files);
  assets->files.max_size = assets_count;
}
