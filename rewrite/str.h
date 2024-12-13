//
// Some standard library replacement string stuff
//

inline u32 get_length(const char *string) {
  if (string == 0)
  return 0;
    
  u32 length = 0;
  const char *ptr = string;
  while(*ptr != 0) {
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