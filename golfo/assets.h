#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

/*
  Asset Manager
*/

enum Asset_Types {
  AT_SHADER,
  AT_TEXTURE,
  AT_FONT,
  AT_ATLAS,
  AT_GEOMETRY,
  AT_MTLLIB,

  AT_COUNT
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
  "../assets/fonts/",
  "../assets/atlases/",
  "../assets/geometry/",
  "", // mtllibs in .obj / geometry
};

const u32 asset_size[] = {
  sizeof(Shader), 
  sizeof(Texture), 
  //sizeof(Font), 
  //sizeof(Texture_Atlas),
  //sizeof(Geometry),
  //sizeof(Material_Library),
};

struct Asset_Array {
  Buffer buffer;
  u32 count;
  u32 type_size;

  u32 insert_index; // if insert functions are being used

  inline void* find(u32 id) {
    return (void*)((char*)buffer.memory + (id * type_size));
  }
};

internal void 
prepare_asset_array(Asset_Array *arr, u32 count, u32 size) {
  if (arr->count == 0) {
    arr->count = count;
    arr->type_size = size;
    arr->buffer = blank_buffer(arr->count * size);
  } else {
    arr->buffer.clear();
  }
}

internal void
asset_array_resize(Asset_Array *arr) {
  ASSERT(arr->type_size != 0);

  if (arr->count == 0) {
    prepare_asset_array(arr, 1, arr->type_size);
  } else {
    arr->count *= 2;
    buffer_resize(&arr->buffer, arr->count * arr->type_size);
  }
}

/*
DOES NOT WORK CORRECTLY

internal void
assert_array_insert(Asset_Array *arr, void *n) {
  ASSERT(arr->count > arr->insert_index);
  void *insert_ptr = (void*)((char*)arr->buffer.memory + (arr->insert_index * arr->type_size));
  if (arr->buffer.in(insert_ptr)) {
    memcpy(insert_ptr, n, arr->type_size);
    arr->insert_index++;
  }  

  log_error("assert_array_insert(): not enough room to insert\n");
  ASSERT(0);
}
*/

internal void*
asset_array_next(Asset_Array *arr) {
  if (arr->insert_index >= arr->count) {
    asset_array_resize(arr);
  }  

  void *insert_ptr = (void*)((char*)arr->buffer.memory + (arr->insert_index++ * arr->type_size));
  if (!arr->buffer.in(insert_ptr)) {
    app_log_error("assert_array_next(): not enough room to insert\n");
    ASSERT(0);
  }
  return insert_ptr;
}

struct Assets {
  Asset_Array pipelines;
  Asset_Array fonts;
  Asset_Array textures;
  Asset_Array atlases;
  Asset_Array geometrys;
  Asset_Array mtllibs;
};

struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

struct Assets_Load_Info {
    Asset_Array *asset_array;
    Asset_Load *loads;
    u32 loads_count;
    u32 asset_type;
};

internal void texture_convert_channels(Texture *bitmap, u32 new_channels);

#endif // ASSETS_LOADER_H
