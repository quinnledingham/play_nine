struct OpenGL_Mesh {
    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ebo; // element array buffer object (index buffer object)
};

struct OpenGL_Info {
    SDL_Window *sdl_window;

    u32 msaa_samples;

    u32 framebuffer;
    u32 texture_color_buffer_multisampled;
    u32 rbo;
    u32 intermediate_fbo;
    u32 screen_texture;
};

OpenGL_Info opengl_info = {};

// lines up with enum shader_stages
const u32 opengl_shader_file_types[5] = { 
    GL_VERTEX_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER,
};
