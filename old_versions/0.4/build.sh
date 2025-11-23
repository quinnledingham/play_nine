CF="-I../ -DDEBUG"

mkdir -p build

cd build

g++ -o golf ${CF} ../golf_sdl.cpp -lSDL3 -lSDL3_ttf

cd ..
