inline u32 
get_length(const char *string) {
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

inline u32 
str_length(const char *str) {
  if (str == 0)
    return 0;
    
  u32 length = 0;
  const char *ptr = str;
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

  String() {
    memory = 0;
    size = 0;
    length = 0;
  }

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
    /*
    if (memory) {
      log_warning("~String(): Not correctly freeing string -> %s\n", memory);
    }

    destroy();
    */
  }
  #endif // DEBUG
};

String::String(const char *a, const char *b) {
  u32 a_length = str_length(a);
  u32 b_length = str_length(b);

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

inline void
float_to_string(float32 f, char *buffer, u32 buffer_size) {
  u32 ret = snprintf(buffer, buffer_size, "%f", f);
  if (ret < 0) {
      log_error("float_to_char_array(float32 f, char *buffer, u32 buffer_size) ftos() failed\n");
      return;
  }
  if (ret >= buffer_size) 
    log_error("float_to_char_array(float32 f, char *buffer, u32 buffer_size) ftos(): result was truncated\n");
}

// ptr must point to first char of int
inline const char*
char_array_to_s32(const char *ptr, s32 *result) {
    u32 sign = 1;
    s32 num = 0;

    if (*ptr == '-') {
        sign = -1;
        ptr++;
    }

    while (isdigit(*ptr)) num = 10 * num + (*ptr++ - '0');
    *result = sign * num;

    return ptr;
}

inline const char*
char_array_to_u32(const char *ptr, u32 *result)
{   
    s32 num = 0;
    ptr = char_array_to_s32(ptr, &num);
    *result = (u32)num;
    return ptr;
}

// char_array_to_float

#define MAX_POWER 20

global const
double POWER_10_POS[MAX_POWER] =
{
    1.0e0,  1.0e1,  1.0e2,  1.0e3,  1.0e4,  1.0e5,  1.0e6,  1.0e7,  1.0e8,  1.0e9,
    1.0e10, 1.0e11, 1.0e12, 1.0e13, 1.0e14, 1.0e15, 1.0e16, 1.0e17, 1.0e18, 1.0e19,
};

global const
double POWER_10_NEG[MAX_POWER] =
{
    1.0e0,   1.0e-1,  1.0e-2,  1.0e-3,  1.0e-4,  1.0e-5,  1.0e-6,  1.0e-7,  1.0e-8,  1.0e-9,
    1.0e-10, 1.0e-11, 1.0e-12, 1.0e-13, 1.0e-14, 1.0e-15, 1.0e-16, 1.0e-17, 1.0e-18, 1.0e-19,
};

inline bool8
is_exponent(char c)
{
    return (c == 'e' || c == 'E');
}

// returns the point where it stopped reading chars
// reads up until it no longer looks like a number
// writes the result it got to where result pointer
inline const char*
char_array_to_float32(const char *ptr, float32 *result)
{
    float64 sign = 1.0;
    float64 num  = 0.0;
    float64 fra  = 0.0;
    float64 div  = 1.0;
    u32 eval = 0;
    const float64* powers = POWER_10_POS;

    switch (*ptr)
    {
        case '+': sign =  1.0; ptr++; break;
        case '-': sign = -1.0; ptr++; break;
    }

    while (isdigit(*ptr)) num = 10.0 * num + (double)(*ptr++ - '0');

    // @WARNING . or , based on system keyboard
    if (*ptr == '.' || *ptr == ',') ptr++;

    while (isdigit(*ptr))
    {
        fra  = 10.0 * fra + (double)(*ptr++ - '0');
        div *= 10.0;
    }

    num += fra / div;

    if (is_exponent(*ptr))
    {
        ptr++;

        switch (*ptr)
        {
            case '+': powers = POWER_10_POS; ptr++; break;
            case '-': powers = POWER_10_NEG; ptr++; break;
        }

        while (isdigit(*ptr)) eval = 10 * eval + (*ptr++ - '0');

        num *= (eval >= MAX_POWER) ? 0.0 : powers[eval];
    }

    *result = (float32)(sign * num);

    return ptr;
}