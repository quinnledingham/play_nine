internal u32
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

// assumes that dest is big enough for src
inline void
copy_str(char *dest, const char *src)
{
    const char *ptr = src;
    while(*ptr != 0)
    {
        *dest++ = *ptr;
        ptr++;
    }
}

inline u32
s32_to_str(char *buffer, u32 size, s32 in) {
    u32 ret = snprintf(buffer, size, "%d", in);
    if (ret < 0) {
        logprint("s32_to_str()", "snprintf() failed\n");
        return 0;
    }
    if (ret >= size)
        logprint("s32_to_str()", "snprintf(): result was truncated\n");
    return ret;
}

inline const char*
s32_to_str(s32 in) {
    u32 size = 10;
    char *buffer = (char*)iru_malloc(size);
    iru_memset(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%d", in);
    if (ret < 0) {
        logprint("s32_to_str()", "snprintf() failed\n");
        return 0;
    }
    if (ret >= size) logprint("s32_to_str()", "snprintf(): result was truncated\n");
    return buffer;
}

inline u32
u32_to_str(char *buffer, u32 size, u32 in) {
    u32 ret = snprintf(buffer, size, "%u", in);
    if (ret < 0) {
        logprint("u32_to_str()", "snprintf() failed\n");
        return 0;
    }
    if (ret >= size)
        logprint("u32_to_str()", "snprintf(): result was truncated\n");
    return ret;
}

inline const char*
u32_to_str(u32 in) {
    return u32_to_str(in);
}

inline const char *
float_to_str(float32 f) {
    u32 size = 64;
    char *buffer = (char*)iru_malloc(size);
    iru_memset(buffer, 0, size);
    u32 ret = snprintf(buffer, size, "%f", f);
    if (ret < 0) {
        logprint("float_to_char_array()", "ftos() failed\n");
        return 0;
    }
    if (ret >= size) logprint("float_to_char_array(float32 f)", "ftos(): result was truncated\n");
    return buffer;
}

inline void
float_to_str(float32 f, char *buffer, u32 buffer_size) {
    u32 ret = snprintf(buffer, buffer_size, "%f", f);
    if (ret < 0) {
        logprint("float_to_char_array(float32 f, char *buffer, u32 buffer_size)", "ftos() failed\n");
        return;
    }
    if (ret >= buffer_size) logprint("float_to_char_array(float32 f, char *buffer, u32 buffer_size)", "ftos(): result was truncated\n");
}
