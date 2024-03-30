internal void
opengl_msaa(u32 samples, u32 width, u32 height) {
    u32 msaa;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, msaa);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);  
}

void opengl_sdl_init(SDL_Window *sdl_window) {

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
    print("OpenGL loaded:\n");
    print("Vendor:   %s\n", glGetString(GL_VENDOR));
    print("Renderer: %s\n", glGetString(GL_RENDERER));
    print("Version:  %s\n", glGetString(GL_VERSION));

    opengl_info.sdl_window = sdl_window;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_FRAMEBUFFER_SRGB); 
    glPointSize(4.0f);
    glEnable(GL_MULTISAMPLE);  

    glEnable(GL_CULL_FACE);  
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);  

    //glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
}

void opengl_clear_color(Vector4 color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void opengl_start_frame() {
    u32 gl_clear_flags = 
            GL_COLOR_BUFFER_BIT  | 
            GL_DEPTH_BUFFER_BIT  | 
            GL_STENCIL_BUFFER_BIT;

    glClear(gl_clear_flags);
}

void opengl_end_frame() {
    SDL_GL_SwapWindow(opengl_info.sdl_window);
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

void opengl_create_descriptor_sets(Descriptor_Set *set, Shader *shader, u32 descriptor_set_count, u32 pool_index) {

}

// block index is from glUniformBlockBinding or binding == #
void opengl_init_ubo(Descriptor_Set *set, Descriptor *descriptor, u32 size, u32 binding) {
    descriptor->binding = binding; // binding point in buffer (offset)
    descriptor->size = size;
    descriptor->handle = platform_malloc(sizeof(u32));
    glGenBuffers(1, (u32*)descriptor->handle);
    
    // clearing buffer
    glBindBuffer(GL_UNIFORM_BUFFER, *(u32*)descriptor->handle);
    glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    //glBindBufferBase(GL_UNIFORM_BUFFER, binding, *(u32*)descriptor->handle); // binding index refers to the memory
}

void opengl_create_descriptor_pool(Shader *shader, u32 descriptor_set_count, u32 set_index) {
    for (u32 i = 0; i < shader->max_sets; i++) {
        for (u32 j = 0; j < shader->layout_sets[set_index].descriptors_count; j++) {
            shader->descriptor_sets[set_index][i].descriptors_count = shader->layout_sets[set_index].descriptors_count;
            shader->descriptor_sets[set_index][i].descriptors[j].type = shader->layout_sets[set_index].descriptors[j].type;
            if (shader->layout_sets[set_index].descriptors[j].type == DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                opengl_init_ubo(&shader->descriptor_sets[set_index][i], 
                                &shader->descriptor_sets[set_index][i].descriptors[j], 
                                shader->layout_sets[set_index].descriptors[j].size,
                                shader->layout_sets[set_index].descriptors[j].binding);
        }
    }
}

void opengl_create_graphics_pipeline(Render_Pipeline *pipeline, Vertex_Info vertex_info) {
/*
    for (u32 i = 0; i < shader->layout_count; i++) {
        for (u32 j = 0; j < shader->descriptor_sets[i].descriptors_count; j++) {
            Descriptor *d = &shader->descriptor_sets[i].descriptors[j];
            if (d->type == DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                opengl_init_ubo(d, d->size, d->binding); 
        }
    }
*/
}

void opengl_pipeline_cleanup(Render_Pipeline *pipe) {

}

Descriptor_Set* opengl_get_descriptor_set(Shader *shader, bool8 layout_index) {

    u32 next_set = shader->sets_count[layout_index]++;

    if (next_set > shader->max_sets) {
        logprint("opengl_get_descriptor_set()", "ran out of sets to use in shader\n");
        ASSERT(0);
        return 0;
    }

    return &shader->descriptor_sets[layout_index][next_set];

}

// returns the new offset
u32 opengl_buffer_sub_data(u32 target, u32 offset, u32 size, void *data) {
    glBufferSubData(target, offset, size, data);
    return (offset + size);
}

#define BUFFER_SUB_DATA(target, offset, n) opengl_buffer_sub_data(target, offset, sizeof(n), (void *)&n)

void opengl_update_ubo(Descriptor_Set *set, u32 descriptor_index, void *data, bool8 static_update) {
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

/*  glBindBuffer(target, *(u32*)ubo->handle);
    offset = BUFFER_SUB_DATA(target, offset, matrices.model);
    offset = BUFFER_SUB_DATA(target, offset, matrices.view);
    offset = BUFFER_SUB_DATA(target, offset, matrices.projection);
    glBindBuffer(target, 0);*/

    Descriptor *descriptor = &set->descriptors[descriptor_index];
    glBindBuffer(target, *(u32*)descriptor->handle);
//  offset = BUFFER_SUB_DATA(target, offset, data);
    glBufferSubData(target, offset, descriptor->size, data);
    glBindBuffer(target, 0);
}

void opengl_update_ubo_v2(Descriptor *descriptor, void *data, u32 offset) {
    GLenum target = GL_UNIFORM_BUFFER;

    glBindBuffer(target, *(u32*)descriptor->handle);
    glBufferSubData(target, offset, sizeof(Object), data);
    glBindBuffer(target, 0);
}

// this is where I am connect memory to a point in the shader
void opengl_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index) {
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

void opengl_bind_descriptor_set(Descriptor_Set *set, u32 first_set) {
    for (u32 i = 0; i < set->descriptors_count; i++) {
        switch(set->descriptors[i].type) {
            case DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                opengl_set_uniform_block_binding(global_shader_handle, "scene", 0); 

                glBindBufferBase(GL_UNIFORM_BUFFER, set->descriptors[i].binding, *(u32*)set->descriptors[i].handle);
            } break;

            case DESCRIPTOR_TYPE_SAMPLER: {
                //glActiveTexture(GL_TEXTURE0 + i);
                //glBindTexture(GL_TEXTURE_2D, *(u32*)set->descriptors[i].handle);
            } break;
        }
    }
}

void opengl_push_constants(Descriptor_Set *push_constants, void *data) {
    GLint location = glGetUniformLocation(global_shader_handle, "object.model");
    glUniformMatrix4fv(location, (GLsizei)1, false, (GLfloat *)data);
}

void opengl_reset_descriptor_sets(Assets *assets) {
    Asset_Array *array = &assets->types[ASSET_TYPE_SHADER];
    for (u32 i = 0; i < array->num_of_assets; i++) {
        Shader *shader = (Shader *)&array->data[i].memory;
        for (u32 layout_i = 0; layout_i < shader->layout_count; layout_i++) {
            shader->sets_count[layout_i] = 0;
        }
    }
}

void opengl_assets_cleanup(Assets *assets) {

}

void opengl_wait_frame() {

}

//
// Shaders
//

bool compile_shader(u32 handle, const char *file, int type)
{
    u32 shader =  glCreateShader((GLenum)type);
    glShaderSource(shader, 1, &file, NULL);
    glCompileShader(shader);
    
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

// compiles the files
void opengl_compile_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->files[0].memory == 0) {
        logprint("compile_shader(Shader *shader)", "vertex shader required");
        return;
    }

    for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
        if (shader->files[i].memory == 0) continue; // file was not loaded

        shaderc_compiler_t compiler = shaderc_compiler_initialize();
        File spv_file = compile_glsl_to_spv(compiler, &shader->files[i], (shaderc_shader_kind)shaderc_glsl_file_types[i]);
        File glsl_file = compile_spv_to_glsl(&spv_file);

        if (!compile_shader(shader->handle, (char*)glsl_file.memory, opengl_shader_file_types[i])) {
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
        logprint("compile_shader()", "link failed");
        return;
    }

    shader->compiled = true;
}

u32 use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
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

void opengl_set_bitmap(Descriptor_Set *set, Bitmap *bitmap, u32 binding) {
    glActiveTexture(GL_TEXTURE0 + binding);
    glBindTexture(GL_TEXTURE_2D, *(u32*)bitmap->gpu_info);
}

internal void
opengl_delete_texture(Bitmap *bitmap) {
    glDeleteTextures(1, (u32*)bitmap->gpu_info);
}

void opengl_depth_test(bool32 enable) {
    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}