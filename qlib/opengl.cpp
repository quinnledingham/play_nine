internal void
opengl_msaa(u32 samples, u32 width, u32 height) {
    u32 msaa;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);  
}

bool8 opengl_sdl_init(SDL_Window *sdl_window) {

    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,    1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_SetSwapInterval(0); // vsync: 0 off, 1 on
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    print("OpenGL loaded:\n");                         // (vulkan device property)
    print("Vendor:   %s\n", glGetString(GL_VENDOR));   // i.e. NVIDIA Corporation (vendorID)
    print("Renderer: %s\n", glGetString(GL_RENDERER)); // Graphics Card Name (deviceName)
    print("Version:  %s\n", glGetString(GL_VERSION));  // (driverVersion)

    opengl_info.sdl_window = sdl_window;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_FRAMEBUFFER_SRGB); 
    glPointSize(4.0f);
    glEnable(GL_MULTISAMPLE);  

    //glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);  

    //glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);

    return 0;
}

void opengl_clear_color(Vector4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

bool8 opengl_start_frame() {
    u32 gl_clear_flags = 
            GL_COLOR_BUFFER_BIT  | 
            GL_DEPTH_BUFFER_BIT  | 
            GL_STENCIL_BUFFER_BIT;

    glClear(gl_clear_flags);

    return 0;
}

void opengl_end_frame(Assets *assets, App_Window *window) {
    SDL_GL_SwapWindow(opengl_info.sdl_window);
}

void opengl_graphics_pipeline_cleanup(Render_Pipeline *pipe) {

}

void opengl_compute_pipeline_cleanup(Compute_Pipeline *pipe) {

}

void opengl_cleanup() {

}

void opengl_init_mesh(Mesh *mesh) {

    OpenGL_Mesh *gl_mesh = (OpenGL_Mesh*)platform_malloc(sizeof(OpenGL_Mesh));

    // allocating buffer
    glGenVertexArrays(1, &gl_mesh->vao);
    glGenBuffers(1, &gl_mesh->vbo);
    glGenBuffers(1, &gl_mesh->ebo);
    
    glBindVertexArray(gl_mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl_mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh->ebo);

    // defining a vertex
    if (mesh->vertex_info.size == sizeof(Vertex_XNU)) {
        glEnableVertexAttribArray(0); // vertex positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)offsetof(Vertex_XNU, position));
        glEnableVertexAttribArray(1); // vertex color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)offsetof(Vertex_XNU, normal));
        glEnableVertexAttribArray(2); // vertex texture coords
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XNU), (void*)offsetof(Vertex_XNU, uv));
    } else if (mesh->vertex_info.size == sizeof(Vertex_XU)) {
        glEnableVertexAttribArray(0); // vertex positions
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XU), (void*)offsetof(Vertex_XU, position));
        glEnableVertexAttribArray(1); // vertex color
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_XU), (void*)offsetof(Vertex_XU, uv));
    }

    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex_XNU), mesh->vertices, GL_STATIC_DRAW);  
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), mesh->indices, GL_STATIC_DRAW);
    
    glBindVertexArray(0);

    mesh->gpu_info = (void *)gl_mesh;
}

void opengl_draw_mesh(Mesh *mesh)
{
    OpenGL_Mesh *gl_mesh = (OpenGL_Mesh *)mesh->gpu_info;
    glBindVertexArray(gl_mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void opengl_set_viewport(u32 window_width, u32 window_height) {
    glViewport(0, 0, window_width, window_height);    
}

void opengl_set_scissor(s32 x, s32 y, u32 window_width, u32 window_height) {
    //v2s bottom_left, dim;
    //glScissor(bottom_left.x, bottom_left.y, dim.x, dim.y);
    glScissor(0, 0, window_width, window_height);
}

global u32 global_shader_handle;

void opengl_bind_pipeline(Render_Pipeline *pipeline) {
    Shader *shader = pipeline->shader;

    glUseProgram(shader->handle);
    //return shader->handle;
    global_shader_handle = shader->handle;
/*
    if (pipeline->depth_test)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
*/
}

// block index is from glUniformBlockBinding or binding == #
void opengl_init_ubo(u32 *handle, Layout_Binding *layout_binding, u32 num_of_sets, u32 offsets[128]) {
    glGenBuffers(1, handle);
    
    // clearing buffer
    glBindBuffer(GL_UNIFORM_BUFFER, *handle);
    glBufferData(GL_UNIFORM_BUFFER, layout_binding->size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //glBindBufferBase(GL_UNIFORM_BUFFER, binding, *(u32*)descriptor->handle); // binding index refers to the memory
}

void opengl_init_layout_offsets(Layout *layout, Bitmap *bitmap) {
    if (layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER || layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        //opengl_init_ubo(&layout->handle, &layout->bindings[0], layout->max_sets, layout->offsets);

        for (u32 i = 0; i < layout->max_sets; i++) {
            opengl_init_ubo(&layout->handles[i], &layout->bindings[0], layout->max_sets, layout->offsets); 
        }
    }

    if (layout->bindings[0].descriptor_type == DESCRIPTOR_TYPE_SAMPLER) {
        //vulkan_init_bitmaps(layout->descriptor_sets, layout->max_sets, bitmap, &layout->bindings[0]);
    }


}

void opengl_create_graphics_pipeline(Render_Pipeline *pipeline, Vertex_Info vertex_info) {

}

void opengl_pipeline_cleanup(Render_Pipeline *pipe) {

}

Descriptor opengl_get_descriptor_set(Layout *layout) {

    if (layout->sets_in_use + 1 > layout->max_sets)
        ASSERT(0);

    u32 return_index = layout->sets_in_use++;
    //if (vulkan_info.current_frame == 1)
        //return_index += layout->max_sets / 2;

    Descriptor desc = {};
    desc.binding = layout->bindings[0];
    desc.offset = layout->offsets[return_index];
    desc.set_number = layout->set_number;
    desc.handle = &layout->handles[return_index];
    desc.bitmaps = layout->bitmaps[return_index];
    desc.bitmaps_saved = &layout->bitmaps_saved[return_index];
    //desc.vulkan_set = &layout->descriptor_sets[return_index];

    return desc;

}

Descriptor opengl_get_descriptor_set_index(Layout *layout, u32 return_index) {
    if (layout->sets_in_use + 1 > layout->max_sets)
        ASSERT(0);

    if (return_index < layout->sets_in_use) {
        logprint("vulkan_get_descriptor_set()", "descriptor could already be in use\n");
    } else {
        layout->sets_in_use = return_index + 1; // jump to after this set
    }

    Descriptor desc = {};
    desc.binding = layout->bindings[0];
    desc.offset = layout->offsets[return_index];
    desc.set_number = layout->set_number;
    desc.handle = &layout->handles[return_index];
    desc.bitmaps = layout->bitmaps[return_index];
    desc.bitmaps_saved = &layout->bitmaps_saved[return_index];

    return desc;
}

// returns the new offset
u32 opengl_buffer_sub_data(u32 target, u32 offset, u32 size, void *data) {
    glBufferSubData(target, offset, size, data);
    return (offset + size);
}

#define BUFFER_SUB_DATA(target, offset, n) opengl_buffer_sub_data(target, offset, sizeof(n), (void *)&n)

void opengl_update_ubo(Descriptor desc, void *data) {
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

/*  glBindBuffer(target, *(u32*)ubo->handle);
    offset = BUFFER_SUB_DATA(target, offset, matrices.model);
    offset = BUFFER_SUB_DATA(target, offset, matrices.view);
    offset = BUFFER_SUB_DATA(target, offset, matrices.projection);
    glBindBuffer(target, 0);*/

    glBindBuffer(target, *desc.handle);
//  offset = BUFFER_SUB_DATA(target, offset, data);
    glBufferSubData(target, offset, desc.binding.size, data);
    glBindBuffer(target, 0);
}

// this is where I am connect memory to a point in the shader
void opengl_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index) {
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

void opengl_bind_descriptor_set(Descriptor desc) {
    switch(desc.binding.descriptor_type) {
        case DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
            opengl_set_uniform_block_binding(global_shader_handle, "scene", 0); 

            glBindBufferBase(GL_UNIFORM_BUFFER, desc.binding.binding, *(u32*)desc.handle);
            glBindBuffer(GL_UNIFORM_BUFFER, *desc.handle);
        } break;

        case DESCRIPTOR_TYPE_SAMPLER: {
            //glActiveTexture(GL_TEXTURE0 + );
            //glBindTexture(GL_TEXTURE_2D, *(u32*)desc.handle);
            for (u32 i = 0; i < *desc.bitmaps_saved; i++) {
                if (desc.bitmaps[i] == 0) continue;
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, *(u32*)desc.bitmaps[i]->gpu_info);
            }

            s32 values[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15  };
            glUniform1iv(glGetUniformLocation(global_shader_handle, "texSampler"), 16, values);

        } break;
    }
}

void opengl_push_constants(u32 shader_stage, void *data, u32 data_size) {
    Object *object = (Object *)data;

    GLint location = glGetUniformLocation(global_shader_handle, "object.model");
    glUniformMatrix4fv(location, (GLsizei)1, false, (GLfloat *)&object->model);

    GLint location2 = glGetUniformLocation(global_shader_handle, "object.index");
    glUniform1i(location2, (GLint)object->index);
}

void opengl_assets_cleanup(Assets *assets) {

}

void opengl_wait_frame() {

}

u32 opengl_set_bitmap(Descriptor *desc, Bitmap *bitmap) {
    u32 index = desc->texture_index++;
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, *(u32*)bitmap->gpu_info);

    desc->bitmaps[index] = bitmap;
    *desc->bitmaps_saved = desc->texture_index;

    return index;
}

void opengl_delete_texture(Bitmap *bitmap) {
    glDeleteTextures(1, (u32*)bitmap->gpu_info);
}

void opengl_depth_test(bool32 enable) {
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void opengl_create_set_layout(Layout *layout) {

}

void opengl_allocate_descriptor_set(Layout *layout) {

}

void opengl_bind_descriptor_sets(Descriptor desc, u32 first_set, void *data, u32 size) {

}

void opengl_create_compute_pipeline(Compute_Pipeline *pipeline) {

}

//
// Shaders
//

/* Can't use glUniform I guess
https://www.geeks3d.com/20200211/how-to-load-spir-v-shaders-in-opengl/

Can't really use push constants then
*/

bool opengl_attach_spirv_shader(u32 handle, const char *file, u32 file_length, int type) {
    u32 shader =  glCreateShader((GLenum)type);
    glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, file, file_length);
    glSpecializeShader(shader, "main", 0, 0, 0);
    
    GLint compiled_shader = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled_shader);  
    if (!compiled_shader) {
        opengl_debug(GL_SHADER, shader);
    } else {
        glAttachShader(handle, shader);
    }
    
    glDeleteShader(shader);
    
    return compiled_shader;
}

internal bool8
opengl_compile_glsl(const char *file, s32 type) {
    u32 shader =  glCreateShader((GLenum)type);
    glShaderSource(shader, 1, &file, NULL);
    glCompileShader(shader);
    
    return shader;
}

bool opengl_attach_glsl_shader(u32 handle, u32 new_part) {

    GLint compiled_shader = 0;
    glGetShaderiv(new_part, GL_COMPILE_STATUS, &compiled_shader);  

    if (!compiled_shader) {
        opengl_debug(GL_SHADER, new_part);
    } else {
        glAttachShader(handle, new_part);
    }
    
    glDeleteShader(new_part);
    
    return compiled_shader;
}

// compiles the files
void opengl_compile_shader(Shader *shader) {
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->files[0].memory == 0) {
        logprint("compile_shader(Shader *shader)", "vertex shader required\n");
        return;
    }

    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->spirv_files[i].memory == 0) continue; // file was not loaded

        File glsl_file = compile_spirv_to_glsl(&shader->spirv_files[i]);
        glsl_file.filepath = shader->files[i].filepath;

        u32 new_part = opengl_compile_glsl((char*)glsl_file.memory, opengl_shader_file_types[i]);

        if (!opengl_attach_glsl_shader(shader->handle, new_part)) {
            print("compile_shader() could not compile %s\n", shader->files[i].filepath); 
            return;
        }
    }

    // Link
    glLinkProgram(shader->handle);

    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program) {
        opengl_debug(GL_PROGRAM, shader->handle);
        logprint("compile_shader()", "link failed (%s)\n", shader->files[SHADER_STAGE_FRAGMENT].filepath);
        return;
    }

    shader->compiled = true;
}

//
// Textures
//

internal void
init_bitmap_gpu_handle(Bitmap *bitmap, u32 texture_parameters) {
    GLenum target = GL_TEXTURE_2D;
    
    bitmap->gpu_info = platform_malloc(sizeof(u32));
    glGenTextures(1, (u32*)bitmap->gpu_info);
    glBindTexture(target, *(u32*)bitmap->gpu_info);
    
    GLint internal_format = 0;
    GLenum data_format = 0;
    GLint pixel_unpack_alignment = 0;
    
    switch(bitmap->channels) {
        case 1: {
            internal_format = GL_RED,
            data_format = GL_RED,
            pixel_unpack_alignment = 1; 
        } break;

        case 3: {
            internal_format = GL_RGB;
            data_format = GL_RGB;
            pixel_unpack_alignment = 1; // because RGB is weird case unpack alignment can't be 3
        } break;
        
        case 4: {
            internal_format = GL_SRGB8_ALPHA8; //GL_RGBA no sRGB
            data_format = GL_RGBA;
            pixel_unpack_alignment = 4;
        } break;
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
    glTexImage2D(target, 0, internal_format, bitmap->width, bitmap->height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    glGenerateMipmap(target);
    
    switch(texture_parameters)
    {
        case TEXTURE_PARAMETERS_DEFAULT:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

        case TEXTURE_PARAMETERS_CHAR:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
    
    glBindTexture(target, 0);
}

void opengl_create_texture(Bitmap *bitmap, u32 texture_parameters) { 
    init_bitmap_gpu_handle(bitmap, texture_parameters); 
}
