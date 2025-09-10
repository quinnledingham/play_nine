#ifdef OS_WINDOWS

inline DWORD 
win32_get_file_stream(u32 output_stream) {
  switch(output_stream) {
    case OUTPUT_DEFAULT: return STD_OUTPUT_HANDLE;
    case OUTPUT_WARNING: return STD_ERROR_HANDLE;
    case OUTPUT_ERROR:   return STD_ERROR_HANDLE;
    default:             return STD_OUTPUT_HANDLE;
  }
}

inline void 
print_char_array(u32 output_stream, const char *char_array) {
  OutputDebugStringA((LPCSTR)char_array);

  HANDLE output = GetStdHandle(win32_get_file_stream(output_stream));
  if (output != NULL && output != INVALID_HANDLE_VALUE) {
    WriteConsole(output, (VOID *)char_array, get_length(char_array), NULL, NULL);
  }
}

#endif // OS

internal void 
output_list(u32 output_stream, const char *msg, va_list list) {
  SDL_LockMutex(output_buffer_mutex);

  #if DEBUG
  // Check to see if the output buffer is initialized before using it
  if (output_buffer.memory == 0 || output_buffer.size == 0) {
    init_output_buffer();
    sprintf_s(output_buffer.str(), output_buffer.size, "(WARNING) (output_list) Not initializing output buffer before use\n");
    print_char_array(output_stream, output_buffer.str());
  }
  #endif // DEBUG

  u32 msg_size = str_length(msg);

  va_list args_copy;
  va_copy(args_copy, list);

  output_buffer.clear();
  s32 result = vsnprintf (output_buffer.str(), (size_t)output_buffer.size, msg, args_copy);

  print_char_array(output_stream, output_buffer.str());

  if (result > (s32)output_buffer.size) {
    output_buffer.clear();
    sprintf_s(output_buffer.str(), output_buffer.size, "\n(WARNING) (output_list) output was concatenated\n");
    print_char_array(output_stream, output_buffer.str());
  }

  SDL_UnlockMutex(output_buffer_mutex);
}
