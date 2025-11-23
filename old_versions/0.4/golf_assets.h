#ifndef GOLF_ASSETS_H
#define GOLF_ASSETS_H

struct File : Buffer {
  String filepath;
};

struct Font {
  TTF_Font *ttf_font;
};

struct Mesh {

};

struct Shader {

};

struct Texture {

};

//
// Assets struct
//

enum Asset_Types {
  AT_SHADER,
  AT_TEXTURE,
  AT_FONT,
  AT_ATLAS,
  AT_GEOMETRY,
  AT_MTLLIB,

  AT_COUNT
};

const char *asset_folders[] = {
  "../assets/shaders/",
  "../assets/bitmaps/",
  "./assets/fonts/",
  "../assets/atlases/",
  "../assets/geometry/",
  "", // mtllibs in .obj / geometry
};

const u32 asset_sizes[] = {
  sizeof(Shader),
  sizeof(Texture),
  sizeof(Font),
  //sizeof(Texture_Atlas),
  //sizeof(Geometry),
  //sizeof(Material_Library),
};

struct Asset_Array {
  Buffer buffer;
  u32 count;
  u32 type_size;

  u32 insert_index; // if insert functions are being used

  inline void* find(u32 id) {
    return (void*)((char*)buffer.memory + (id * type_size));
  }
};

struct Assets {
  Asset_Array pipelines;
  Asset_Array fonts;
  Asset_Array textures;
  Asset_Array atlases;
  Asset_Array geometrys;
  Asset_Array mtllibs;
};

struct Asset_Load {
  u32 id;
  const char *filenames[5];
};

struct Assets_Load_Info {
    Asset_Array *asset_array;
    Asset_Load *loads;
    u32 loads_count;
    u32 asset_type;
};

Assets assets = {};

inline Font*     find_font    (u32 id) { return (Font *)assets.fonts.find(id);         }

//
// Specifying Assets
//

enum Font_Ids {
  FONT_LIMELIGHT,
  FONT_ROBOTO_MONO,

  FONT_COUNT
};

Asset_Load font_loads[] = {
  { FONT_LIMELIGHT, {"Limelight-Regular.ttf"} },
  { FONT_ROBOTO_MONO, {"RobotoMono.ttf"} },
};

Assets_Load_Info fonts_info = {
  &assets.fonts,
  font_loads,
  FONT_COUNT,
  AT_FONT
};

#endif // GOLF_ASSETS_H
