const char *asset_folders[5] = {
    "/assets/bitmaps/",
    "/assets/fonts/",
    "/assets/shaders/",
};

enum Asset_Token_Type {
    ATT_KEYWORD,
    ATT_ID,
    ATT_SEPERATOR,
    ATT_ERROR,
    ATT_WARNING,
    ATT_END
};

const Pair asset_types[ASSET_TYPE_AMOUNT] = {
    { ASSET_TYPE_BITMAP, "BITMAPS" },
    { ASSET_TYPE_SHADER, "SHADERS" },
    { ASSET_TYPE_FONT,   "FONTS"   },
};

struct Asset_Token {
    s32 type;
    const char *lexeme;
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
    if (is_ascii_letter(ch) || is_file_path_ch(ch))
        return true;
    else
        return false;
}

internal Asset_Token
scan_asset_file(File *file, s32 *line_num) {
    X:

    s32 ch;
    while((ch = file_get_char(file)) != EOF && (ch == 32 || ch == 9 || ch == 13));

    switch(ch) {
    case 0:
    case EOF: {
        return { ATT_END, 0 };
    } break;
        
    case '\n': {
        (*line_num)++;
        goto X;
    } break;

    case ':':
    case ',':
    case '|': {
        return { ATT_SEPERATOR, chtos(1, ch) };
    }
        
    default: {
        if (is_valid_start_ch(ch)) {
            s32 length = 0;
            do {
                ch = file_get_char(file);
                length++;
            } while(is_valid_body_ch(ch));
            file_un_char(file);
            const char *sequence = file_copy_backwards(file, length);
            if (pair_is_value(asset_types, ASSET_TYPE_AMOUNT, sequence))
                return { ATT_KEYWORD, sequence };
            return { ATT_ID, sequence };
        }

        logprint("scan_asset_file()", "not a valid ch\n");
    } break;
    }

    return { ATT_ERROR, 0 };
}

struct Asset_Parse_Info {
    s32 type;
    s32 indexes[ASSET_TYPE_AMOUNT];

    const char *tag; // name
    const char *filename;
    const char *path;

    const char *file_paths[SHADER_STAGES_AMOUNT];
};

internal void
parse_asset_file(Assets *assets, File *file, void (action)(void *data, void *args)) {
    Asset_Parse_Info info = {};
    
    u32 shader_type = 0;
    const char *shader_tag = 0;
    
    Asset_Token token = {};
    Asset_Token last_token = {};
    s32 line_num = 1;
    
    while(token.type != ATT_END) {
        last_token = token;
        token = scan_asset_file(file, &line_num);

        switch(token.type) {
        case ATT_KEYWORD: {
            u32 type = pair_get_key(asset_types, ASSET_TYPE_AMOUNT, token.lexeme);
            if (type == ASSET_TYPE_AMOUNT)
                logprint("parse_asset_file()", "cound not find asset type for lexeme\n");

            // add last shader
            if (shader_tag != 0 && type != ASSET_TYPE_SHADER) {
                action((void *)assets, (void *)&info);
                shader_tag = 0;
            }

            info.type = type;

            token = scan_asset_file(file, &line_num, token);
            if (!equal(tok.lexeme, ":")) {
                logprint("parse_asset_file()", "expected ':' (got %c) @ %d\n", token.lexeme, line_num);
                break;
            }
        } break;

        case ATT_ID: {
            if (info.type == ASSET_TYPE_SHADER) {
                if (equal(last_token.lexeme, ",")) {
                    info.file_paths[shader_type] = tok.lexeme;
                } else if (equal(last_token.lexeme, "|")) {
                    shader_type = pair_get_key(shader_types, SHADER_TYPE_AMOUNT, token.lexeme);

                    token = scan_asset_file(file, &line_num, token);
                    if (!equal(token.lexeme, ",")) {
                        logprint("parse_asset_file()", "expected ',' @ %d\n", line_num);
                        break;
                    }
                } else {
                    info.tag = tok.lexeme;

                    if (shader_tag == 0) {

                    } else if (!equal(shader_tag, info.tag)) {

                    }
                }
            } else {
                if (!equal(last_token.lexeme, ",")) {

                } else {

                }
            }
        } break;

        case ATT_SEPERATOR: {
            logprint("parse_asset_file()", "unexpected seperator @ %d\n", line_num);
        } break;
        }
    }
}

internal void
load_asset(void *data, void *args) {
    Assets *assets = (Assets *)data;
}

// returns true if loading the assets fails
internal bool8
load_assets(Assets *assets, const char *filepath) {
    File assets_file = load_file(filepath);
    if (assets_file.size == 0) {
        logprint("load_assets()", "could not open file %s\n", filepath);
        return true;
    }

    s32 line_num = 0;
    Asset_Token token = {};
    do {
        token = scan_asset_file(&assets_file, &line_num);
        if (token.type < 3)
            print("%d %s @ %d\n", token.type, token.lexeme, line_num);
    } while(token.type != ATT_END);

    return false;
}
