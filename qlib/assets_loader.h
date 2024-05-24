#ifndef ASSETS_LOADER_H
#define ASSETS_LOADER_H

const char *asset_folders[5] = {
    "../assets/bitmaps/",
    "../assets/fonts/",
    "../assets/shaders/",
    "../assets/audios/",
    "../assets/objs/"
};

const Pair asset_types[ASSET_TYPE_AMOUNT] = {
    { ASSET_TYPE_BITMAP, "BITMAPS" },
    { ASSET_TYPE_FONT,   "FONTS"   },
    { ASSET_TYPE_SHADER, "SHADERS" },
    { ASSET_TYPE_AUDIO,  "AUDIOS"  },
    { ASSET_TYPE_MODEL,  "MODELS"  },
};

const Pair shader_types[SHADER_STAGES_AMOUNT] = {
    { SHADER_STAGE_VERTEX,                  "VERTEX"     },
    { SHADER_STAGE_TESSELLATION_CONTROL,    "CONTROL"    },
    { SHADER_STAGE_TESSELLATION_EVALUATION, "EVALUATION" },
    { SHADER_STAGE_GEOMETRY,                "GEOMETRY"   },
    { SHADER_STAGE_FRAGMENT,                "FRAGMENT"   },
    { SHADER_STAGE_COMPUTE,                 "COMPUTE"    },
};

enum Asset_Token_Type {
    ATT_KEYWORD,
    ATT_ID,
    ATT_SEPERATOR,
    ATT_ERROR,
    ATT_WARNING,
    ATT_END
};

struct Asset_Token {
    s32 type;
    const char *lexeme;
};

struct Asset_Parse_Info {
    s32 type;

    const char *tag; // name
    const char *filename;
    const char *path;

    const char *file_paths[SHADER_STAGES_AMOUNT];
};

inline bool8
is_ascii(s32 ch) {
    if (ch >= 0 && ch <= 127) return true;
    else                      return false;
}

inline bool8
is_ascii_letter(s32 ch) {
    if      (ch >= 'A' && ch <= 'Z') return true;
    else if (ch >= 'a' && ch <= 'z') return true;
    else                             return false;
}

inline bool8
is_ascii_digit(s32 ch) {
    if (ch >= '0' && ch <= '9') return true;
    else                        return false;
}

inline bool8
is_file_path_ch(s32 ch) {
    switch(ch) {
    case '.':
    case '/':
    case '-':
    case '_':
        return true;
    default:
        return false;
    }
}

inline bool8
is_valid_body_ch(s32 ch) {
    if (is_ascii_letter(ch) || is_ascii_digit(ch) || is_file_path_ch(ch))
        return true;
    else
        return false;
}

inline bool8
is_valid_start_ch(s32 ch) {
    if (is_ascii_letter(ch) || is_ascii_digit(ch) || is_file_path_ch(ch))
        return true;
    else
        return false;
}

#endif // ASSETS_LOADER_H
