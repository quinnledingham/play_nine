mkdir -p build
g++  -I./glad -I./stb -I./ -I./qlib -I$VULKAN_SDK/include -DSDL -DVULKAN -DDEBUG -g -DOS_LINUX qlib/sdl_application.cpp -o build/play_nine -I/usr/include/SDL2 -L$VULKAN_SDK/lib -lSDL2 -lvulkan -lshaderc_combined -lspirv-cross-c-shared -D_REENTRANT
