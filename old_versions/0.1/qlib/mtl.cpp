struct MTL_Token {
    s32 type;
    union
    {
        const char *lexeme;
        float float_num;
        s32 int_num;
    };
    s32 ch;
};

// stores information taken from .mtl file
struct MTL {
    Material *materials; // loaded from the corresponding material file
    u32 materials_count;
};

enum MTL_Token_Type {
    MTL_TOKEN_KEYWORD,
    MTL_TOKEN_NUMBER,
    MTL_TOKEN_STRING,
    MTL_TOKEN_ERROR,
    MTL_TOKEN_EOF
};

enum MTL_SETTINGS {
    MTL_NEWMTL,
    MTL_SPECULAR_EXPONENT,
    MTL_AMBIENT,
    MTL_DIFFUSE,
    MTL_SPECULAR,
    MTL_MAP_DIFFUSE,
};

const Pair material_settings[6] = {
    { MTL_NEWMTL,            "newmtl"},
    { MTL_SPECULAR_EXPONENT, "Ns"    },
    { MTL_AMBIENT,           "Ka"    },
    { MTL_DIFFUSE,           "Kd"    },
    { MTL_SPECULAR,          "Ks"    },
    { MTL_MAP_DIFFUSE,       "map_Kd"},
};

static const s32 mtl_valid_chars[5] = { '-', '.', '_', ':', '/' };

internal bool8
is_valid_mtl_char(s32 ch) {
    for (s32 i = 0; i < ARRAY_COUNT(mtl_valid_chars); i++) {
        if (ch == mtl_valid_chars[i]) return true;
    }
    return false;
}

internal MTL_Token*
create_mtl_token(MTL_Token token) {
    MTL_Token *mtl = (MTL_Token*)platform_malloc(sizeof(MTL_Token));
    (*mtl) = token;
    return mtl;
}

internal void*
scan_mtl(File *file, s32 *line_num, char *buffer, u32 buffer_length) {
    X:
    
    s32 ch;
    while((ch = file_get_char(file)) != EOF && (ch == 9 || ch == 13)); // remove tabs
    //log("%c %d", ch, file->size);
    switch(ch) {
        case EOF: { return (void*)create_mtl_token({ MTL_TOKEN_EOF, 0, ch }); } break;
        
        case '\n': {
            (*line_num)++;
            goto X;
        } break;
        
        case '#': {
            while((ch = file_get_char(file)) != EOF && (ch != '\n'));
            file_un_char(file);
            goto X;
        } break;
        
        case ' ': {
            goto X;
        } break;
        
        default: {
            u32 length = 0;
            buffer[length] = ch;
            do {
                if (length >= buffer_length) return (void*)create_mtl_token({ MTL_TOKEN_ERROR, "mtl over buffer", ch });
                buffer[length++] = ch;
                ch = file_get_char(file);
            } while((isalpha(ch) || isdigit(ch) || is_valid_mtl_char(ch)) && ch != ' ' && ch != EOF);
            buffer[length] = 0;

            if (isalpha(buffer[0])) 
                return (void*)create_mtl_token({ MTL_TOKEN_KEYWORD, buffer });
            return (void*)create_mtl_token({ MTL_TOKEN_NUMBER, buffer });

            logprint("scan_mtl()", "not a valid ch (%d) @ %d\n", ch, line_num);
        } break;
    }
    
    return (void*)create_mtl_token({ MTL_TOKEN_ERROR, 0, ch });
}

internal void
parse_v3(Lexer *lexer, Vector3 *v) {
    MTL_Token *token = 0;
    for (u32 i = 0; i < 3; i++) {
        token = (MTL_Token*)lex(lexer);
        char_array_to_float32(token->lexeme, &v->E[i]);
    }
}

internal MTL
load_mtl(const char *path, const char *filename) {
    MTL mtl = {};
    Lexer lexer = {};
    const char *filepath = char_array_insert(path, get_length(path), filename);
    MTL_Token *token = 0;
    Material material = {};
    u32 material_index = 0;

    // init lexer
    lexer.file = load_file(filepath);
    if (!lexer.file.size) { 
        logprint("load_mtl()", "could not read material file\n"); 
        return mtl; 
    }
    lexer.scan = &scan_mtl;
    lexer.token_size = sizeof(MTL_Token);
    
    // count new materials
    token = (MTL_Token*)lex(&lexer);
    while (token->type != MTL_TOKEN_EOF) {
        if (equal(token->lexeme, "newmtl")) 
            mtl.materials_count++;
        token = (MTL_Token*)lex(&lexer);
    }
    // declare materials memory now that we know how many there are to load
    mtl.materials = ARRAY_MALLOC(Material, mtl.materials_count);
    reset_lex(&lexer);
    
    // fill in mtl.materials array
    token = 0;
    do {
        token = (MTL_Token*)lex(&lexer);
        
        switch(pair_get_key(material_settings, 6, token->lexeme)) {
            case MTL_NEWMTL: {
                if (material.id != 0)
                    mtl.materials[material_index++] = material;

                material = {};
                token = (MTL_Token*)lex(&lexer);
                material.id = token->lexeme;
            } break;

            case MTL_SPECULAR_EXPONENT: {
                token = (MTL_Token*)lex(&lexer);
                char_array_to_float32(token->lexeme, &material.specular_exponent);
            } break;

            case MTL_AMBIENT:  parse_v3(&lexer, &material.ambient);  break;
            case MTL_DIFFUSE:  parse_v3(&lexer, &material.diffuse);  break;
            case MTL_SPECULAR: parse_v3(&lexer, &material.specular); break;

            case MTL_MAP_DIFFUSE: {
                token = (MTL_Token*)lex(&lexer);
                const char *diffuse_map_filepath = char_array_insert(path, get_length(path), token->lexeme);
                File file = load_file(diffuse_map_filepath);
                material.diffuse_map = load_bitmap(file, false);
            } break;
        }
    } while(token->type != MTL_TOKEN_EOF);
    
    if (material.id != 0)
        mtl.materials[material_index++] = material;
    
    ll_free(&lexer.tokens);

    platform_free((void*)filepath);
    free_file(&lexer.file);
    
    return mtl;
}
