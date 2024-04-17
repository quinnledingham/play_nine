mkdir -p build
g++  -I./glad -I./stb -I./ -I./qlib -DSDL -DVULKAN -DDEBUG -DOS_LINUX qlib/sdl_application.cpp -o build/play_nine -I/usr/include/SDL2 -lSDL2 -lvulkan -lshaderc_combined -lspirv-cross-c-shared
