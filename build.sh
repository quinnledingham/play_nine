mkdir -p build

# Set up VULKAN_SDK if it isn't
if [ -z $VULKAN_SDK ]; then
. $HOME/vulkansdk-1.3.290.0/setup-env.sh
fi

# g++  -I./glad -I./stb -I./ -I./qlib -I./steam -I$VULKAN_SDK/include -DSDL -DVULKAN -DDEBUG -DSTEAM -g -DOS_LINUX qlib/sdl_application.cpp -o build/play_nine -I/usr/include/SDL2 -L$VULKAN_SDK/lib -L/home/quinn/SteamSDK/redistributable_bin/linux64 -lSDL2 -lvulkan -lshaderc_combined -lspirv-cross-c-shared -lsteam_api -D_REENTRANT
time g++  -I./glad -I./stb -I./ -I./qlib -I$VULKAN_SDK/include -DSDL -DVULKAN -DDEBUG -g -DOS_LINUX play_nine.cpp -o build/play_nine -I/usr/include/SDL2 -L$VULKAN_SDK/lib -L/home/quinn/SteamSDK/redistributable_bin/linux64 -lSDL2 -lvulkan -lshaderc_combined -lspirv-cross-c-shared -D_REENTRANT
