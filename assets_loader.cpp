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

struct Asset_Token {
    s32 type;
    const char *lexeme;
};
/*
internal Asset_Token
scan_asset_file(File *file, s32 *line_num) {
    X:

    s32 ch;
    while((ch = get_char()) != EOF && (ch == 32 || ch == 9 || ch == 13));

    switch(ch) {
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
            s32 length = 0;
            do {
    
            } while(is_valid_body_ch(ch));
        } break;
    }
}
*/
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
    
    

    return false;
}