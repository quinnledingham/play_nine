struct OpenGL_Mesh {
    u32 vao; // vertex array object
    u32 vbo; // vertex buffer object
    u32 ebo; // element array buffer object (index buffer object)
};

struct OpenGL_Info {
    SDL_Window *sdl_window;
};

OpenGL_Info opengl_info = {};