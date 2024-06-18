#ifdef OS_WINDOWS

internal DWORD
win32_get_file_stream(u32 output_stream) {
	switch(output_stream) {
		case PRINT_DEFAULT: return STD_OUTPUT_HANDLE;
		case PRINT_WARNING: return STD_ERROR_HANDLE;
		case PRINT_ERROR:   return STD_ERROR_HANDLE;
		default:            return STD_OUTPUT_HANDLE;
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

#elif OS_LINUX

internal FILE*
get_file_stream(u32 output_stream) {
	switch(output_stream) {
		case PRINT_DEFAULT: return stdout;
		case PRINT_WARNING: return stderr;
		case PRINT_ERROR:   return stderr;
		default:            return stdout;
	}
}

inline void
print_char_array(u32 output_stream, const char *char_array) {
	fprintf(get_file_stream(output_stream), "%s", char_array);
}

#endif // OS_WINDOWS

#define PRINT_BUFFER_SIZE 2000

internal void
print_list(u32 output_stream, const char *msg, va_list list) {
	char print_buffer[PRINT_BUFFER_SIZE];
	u32 print_buffer_index = 0;

	const char *msg_ptr = msg;
	while(*msg_ptr != 0) {
		if (print_buffer_index >= PRINT_BUFFER_SIZE) {
			print_char_array(PRINT_ERROR, "print_list(): msg to big for print_buffer");
		}

		if (*msg_ptr == '%') {
			msg_ptr++;
			char ch = *msg_ptr;
			u32 length_to_add = 0;
      switch(ch) {
				case 's': {
					const char *string = va_arg(list, const char*);
					length_to_add = get_length(string);
					if (print_buffer_index + length_to_add >= PRINT_BUFFER_SIZE) {
						print_char_array(PRINT_ERROR, "print_list(): msg to big for print_buffer");
					}
					copy_char_array(&print_buffer[print_buffer_index], string);
        } break;
                    
				case 'd': {                    
					int d = va_arg(list, int);                   
					char buffer[20];
					length_to_add = s32_to_char_array(buffer, 20, d);
					copy_char_array(&print_buffer[print_buffer_index], buffer);
				} break;

				case 'u': {
					u32 d = va_arg(list, u32);
					char buffer[20];
					length_to_add = u32_to_char_array(buffer, 20, d);
					copy_char_array(&print_buffer[print_buffer_index], buffer);
				} break;
                    
				case 'f': {
					double f = va_arg(list, double);
					const char *f_string = float_to_char_array((float)f);
					length_to_add = get_length(f_string);
					if (print_buffer_index + length_to_add >= PRINT_BUFFER_SIZE) {
						print_char_array(PRINT_ERROR, "print_list(): msg to big for print_buffer");
					}
					copy_char_array(&print_buffer[print_buffer_index], f_string);
					platform_free((void*)f_string);
				} break;
			}
			print_buffer_index += length_to_add;
		} else {
			print_buffer[print_buffer_index++] = *msg_ptr;
		}
		msg_ptr++;
	}

	// check if there is still room and add terminating zero
	if (print_buffer_index >= PRINT_BUFFER_SIZE) {
		print_char_array(PRINT_ERROR, "print_list(): msg to big for print_buffer");
	}
	print_buffer[print_buffer_index++] = 0;


	print_char_array(output_stream, print_buffer);
}

void print(const char *msg, ...) {
	va_list list;
	va_start(list, msg);
	print_list(PRINT_DEFAULT, msg, list);
	va_end(list);
}

void logprint(const char *where, const char *msg, ...) {
	print_char_array(PRINT_WARNING, where);
	print_char_array(PRINT_WARNING, ": ");

	va_list list;
	va_start(list, msg);
	print_list(PRINT_DEFAULT, msg, list);
	va_end(list);
}
