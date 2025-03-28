enum
{
    OUTPUT_DEFAULT,
    OUTPUT_ERROR,
    OUTPUT_WARNING,
};

#define OUTPUT_LIST(s, m) va_list list; va_start(list, m); output_list(s, m, list); va_end(list);

Buffer output_buffer = {};

void init_output_buffer() {
  output_buffer = blank_buffer(1000);
}

void print_char_array(u32 output_stream, const char *char_array);
void output_list(u32 output_stream, const char *msg, va_list list);

void log_error(const char *msg, ...) {
  print_char_array(OUTPUT_ERROR, "ERROR: ");

  va_list list;
  va_start(list, msg);
  output_list(OUTPUT_ERROR, msg, list);
  va_end(list);
}

void log_warning(const char *msg, ...) {
  print_char_array(OUTPUT_WARNING, "warning: ");

  OUTPUT_LIST(OUTPUT_WARNING, msg);
}

void print(const char *msg, ...) {
  OUTPUT_LIST(OUTPUT_DEFAULT, msg);
}

void log(const char *msg, ...) {
  OUTPUT_LIST(OUTPUT_DEFAULT, msg);
}