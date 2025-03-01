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

struct String {
  void *memory;
  u32 size;
  u32 length;

  String(const char *str) {
    length = get_length(str);
    size = length + 1;
    memory = malloc(size);
    memset(memory, 0, size);
    memcpy(memory, str, length);
  }

  String(const char *a, const char *b);

  const char* str() {
    return (const char *)memory;
  }

  void log() {
    print((char*)memory);
  }

  void remove_ending();

  void destroy() {
    #ifdef DEBUG
    if (memory) {
      free(memory);
    }
    memory = 0;
    size = 0;
    length = 0;
    #else
    free(memory);
    #endif // DEBUG
  }

  #ifdef DEBUG
  ~String() {
    if (memory) {
      log_warning("~String(): Not correctly freeing string -> %s\n", memory);
    }

    destroy();
  }
  #endif // DEBUG
};

String::String(const char *a, const char *b) {
  u32 a_length = get_length(a);
  u32 b_length = get_length(b);

  length = a_length + b_length;
  size = length + 1;

  memory = malloc(size);
  memset(memory, 0, size);
  
  memcpy(memory, a, a_length);
  memcpy(((char *)memory) + a_length, b, b_length);
}

void String::remove_ending() {
  char *ptr = (char *)str();
  ptr += length;
  u32 new_length = length;
  while(*ptr != '.' && ptr > str()) {
    ptr--;
    new_length--;
  }
  while(ptr < str() + length) {
    *ptr = 0;
    ptr++;
  }
  length = new_length;
}