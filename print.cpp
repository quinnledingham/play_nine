enum
{
    PRINT_DEFAULT,
    PRINT_ERROR,
    PRINT_WARNING,
};

internal FILE*
get_file_stream(u32 output_stream) {
	switch(output_stream) {
		case PRINT_DEFAULT: return stdout;
		case PRINT_WARNING: return stderr;
		case PRINT_ERROR:   return stderr;
		default:            return stdout;
	}
}

#ifdef WINDOWS

internal void
print_char_array(u32 output_stream, const char *char_array) {
	OutputDebugStringA((LPCSTR)char_array);
}

#endif // WINDOWS

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
                    char buffer[10];
                    length_to_add = s32_to_char_array(buffer, 10, d);
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

#ifdef OPENGL

void GLAPIENTRY opengl_debug_message_callback(GLenum source, GLenum type, GLuint id,  GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    print("GL CALLBACK:");
    print("message: %s\n", message);
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               print("type: ERROR");               break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: print("type: DEPRECATED_BEHAVIOR"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  print("type: UNDEFINED_BEHAVIOR");  break;
        case GL_DEBUG_TYPE_PORTABILITY:         print("type: PORTABILITY");         break;
        case GL_DEBUG_TYPE_PERFORMANCE:         print("type: PERFORMANCE");         break;
        case GL_DEBUG_TYPE_OTHER:               print("type: OTHER");               break;
    }
    print("id: %d", id);
    switch(severity)
    {
        case GL_DEBUG_SEVERITY_LOW:    print("severity: LOW");    break;
        case GL_DEBUG_SEVERITY_MEDIUM: print("severity: MEDIUM"); break;
        case GL_DEBUG_SEVERITY_HIGH:   print("severity: HIGH");   break;
    }
}

void opengl_debug(int type, int id)
{
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0)
    {
        GLchar info_log[512];
        GLint size;
        
        switch(type)
        {
            case GL_SHADER:  glGetShaderInfoLog (id, 512, &size, info_log); break;
            case GL_PROGRAM: glGetProgramInfoLog(id, 512, &size, info_log); break;
        }
        
        print(info_log);
    }
}

#endif // OPENGL
