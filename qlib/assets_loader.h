#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

const char *asset_folders[5] = {
    "../assets/bitmaps/",
    "../assets/fonts/",
    "../assets/shaders/",
    "../assets/audios/",
    "../assets/objs/"
};

union Asset_Load {
    const char *E[7];
    struct {
      const char *tag;
      const char *vertex;
      const char *control;
      const char *evaluation;
      const char *geometry;
      const char *fragment;
      const char *compute;
    };
    struct {
      const char *tag;
      const char *filename;
    };
};

struct Assets_Load {
    u32 type; // type of the assets in the infos array
    u32 count; 
    Asset_Load *load;
};

#endif // ASSETS_LOADER_H
