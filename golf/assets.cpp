internal File
load_file(const char *filepath) {
  File result = {};

  FILE *in = fopen(filepath, "rb");
  if(in) {
    fseek(in, 0, SEEK_END);
    result.size = ftell(in);
    fseek(in, 0, SEEK_SET);

    result.memory = iru_malloc(result.size);
    fread(result.memory, result.size, 1, in);
    fclose(in);
  } else { 
    logprint("load_file", "Cannot open file %s\n", filepath);
  }

  result.filepath = filepath;
  result.filepath_length = str_length(filepath);
  result.ch = (char*)result.memory;

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
  bitmap.channels = 0;

  // 4 arg always get filled in with the original amount of channels the image had.
  // Currently forcing it to have 4 channels.
  unsigned char *data = stbi_load_from_memory((stbi_uc const *)file.memory, file.size, &bitmap.width, &bitmap.height, &bitmap.channels, 0);
  u32 data_size = bitmap.width * bitmap.height * bitmap.channels;
  bitmap.memory = (u8 *)iru_malloc(data_size);
  iru_memcpy(bitmap.memory, data, data_size);
  stbi_image_free(data);

  if (bitmap.channels != 4) {
    int i = 0;
  }

  if (bitmap.memory == 0) 
    logprint("load_bitmap()", "could not load bitmap %s\n", file.path);

  bitmap.pitch = bitmap.width * bitmap.channels;
  bitmap.mip_levels = (u32)floor(log2f((float32)max(bitmap.width, bitmap.height))) + 1;

  return bitmap;
}
