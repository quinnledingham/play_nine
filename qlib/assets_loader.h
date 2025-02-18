#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

enum Asset_Types {
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_FONT,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_AUDIO,
    ASSET_TYPE_MODEL,
    ASSET_TYPE_ATLAS,

    ASSET_TYPE_AMOUNT
};

const char *asset_folders[ASSET_TYPE_AMOUNT] = {
    "../assets/bitmaps/",
    "../assets/fonts/",
    "../assets/shaders/",
    "../assets/audios/",
    "../assets/objs/",
    "../assets/atlases/",
};

struct Asset_Decl {
    u32 id;
    union {
        const char *E[5];
        const char *filename;
        const char *compute;
        struct {
            const char *vertex;
            const char *control;
            const char *evaluation;
            const char *geometry;
            const char *fragment;
        };
    };
    
};

//
// Assets Loading
//

struct Asset {
    u32 type;
    u32 id;
    File files[5];
    u32 files_count;

    union {
        Bitmap bitmap;
        Font font;
        Shader shader;
        Audio audio;
        Model model;
        Texture_Atlas atlas;

        void *memory;
    };  
};

struct Asset_Array {
    Asset *data;       // points to Assets data array
    u32 num_of_assets;
};

struct Assets {
    bool8 loaded;
    u32 num_of_assets;

    Asset *data;
    Asset_Array types[ASSET_TYPE_AMOUNT];

    u32 find_asset_id(u32 id);
};

internal void*
find_asset(Assets *assets, u32 type, u32 id) {
    for (u32 i = 0; i < assets->types[type].num_of_assets; i++) {
        if (id == assets->types[type].data[i].id)
            return &assets->types[type].data[i].memory;
    }
    logprint("find_asset()", "Could not find asset, type: %d, id: %d\n", type, id);
    return 0;
}

u32 Assets::find_asset_id(u32 id) {
    for (u32 i = 0; i < num_of_assets; i++) {
        if (id == data[i].id)
            return i;
    }
    logprint("find_asset_id()", "Could not find asset, id: %s\n", id);
    return 0;
}

inline Bitmap* find_bitmap(Assets *assets, u32 id) { return (Bitmap*) find_asset(assets, ASSET_TYPE_BITMAP, id); }
inline Font*   find_font  (Assets *assets, u32 id) { return (Font*)   find_asset(assets, ASSET_TYPE_FONT,   id); }
inline Shader* find_shader(Assets *assets, u32 id) { return (Shader*) find_asset(assets, ASSET_TYPE_SHADER, id); }
inline Model*  find_model (Assets *assets, u32 id) { return (Model*)  find_asset(assets, ASSET_TYPE_MODEL,  id); }
inline Audio*  find_audio (Assets *assets, u32 id) { return (Audio*)  find_asset(assets, ASSET_TYPE_AUDIO,  id); }
inline Texture_Atlas*  find_texture_atlas (Assets *assets, u32 id) { return (Texture_Atlas*)  find_asset(assets, ASSET_TYPE_ATLAS, id); }

#endif // ASSETS_LOADER_H
