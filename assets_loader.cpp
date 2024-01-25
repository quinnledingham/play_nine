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

            token = scan_asset_file(file, &line_num);
            if (!equal(token.lexeme, ":")) {
                logprint("parse_asset_file()", "expected ':' (got %c) @ %d\n", token.lexeme, line_num);
                break;
            }
        } break;

        case ATT_ID: {
            if (info.type == ASSET_TYPE_SHADER) {
                if (equal(last_token.lexeme, ",")) {
                    info.file_paths[shader_type] = token.lexeme;
                } else if (equal(last_token.lexeme, "|")) {
                    shader_type = pair_get_key(shader_types, SHADER_STAGES_AMOUNT, token.lexeme);

                    token = scan_asset_file(file, &line_num);
                    if (!equal(token.lexeme, ",")) {
                        logprint("parse_asset_file()", "expected ',' @ %d\n", line_num);
                        break;
                    }
                } else {
                    info.tag = token.lexeme;

                    if (shader_tag == 0) {
                        // first shader being added
                        shader_tag = info.tag;
                    } else if (!equal(shader_tag, info.tag)) {
                        // new shader was started after starting another one
                        const char *new_tag = info.tag;
                        info.tag = shader_tag;
                        action((void*)assets, (void*)&info);

                        int indexes[ASSET_TYPE_AMOUNT];
                        for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) 
                            indexes[i] = info.indexes[i];

                        info = {};
                        info.type = ASSET_TYPE_SHADER;

                        for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) 
                            info.indexes[i] = indexes[i];

                        info.tag = new_tag;
                        shader_tag = info.tag;
                    }

                    // check that | comes after
                    token = scan_asset_file(file, &line_num);
                    if (!equal(token.lexeme, "|")) {
                        logprint("parse_asset_file()", "expected '|' @ %d\n", line_num);
                        break;
                    }
                }
            } else {
                // Fonts, Bitmaps, Audios, Models 
                if (!equal(last_token.lexeme, ",")) {
                    info.tag = token.lexeme;
                    
                    // check thst comma comes after
                    token = scan_asset_file(file, &line_num);
                    if (!equal(token.lexeme, ",")) {
                        logprint("parse_asset_file()", "expected ',' @ %d", line_num);
                        break;
                    }
                } else {
                    info.filename = token.lexeme;
                    action((void*)assets, (void*)&info);
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
load_asset(Asset *asset, Asset_Parse_Info *info) {
    asset->type = info->type;
    asset->tag = info->tag;
    asset->tag_length = get_length(asset->tag);
    //log("loading: %s", asset->tag);

    const char *filename = char_array_concat(asset_folders[asset->type], info->filename);

    // how to load the various assets
    switch(asset->type) {
        case ASSET_TYPE_FONT:   asset->font   = load_font(filename);   break;
        case ASSET_TYPE_BITMAP: asset->bitmap = load_bitmap(filename); break;
            //        case ASSET_TYPE_AUDIO:  asset->audio  = load_audio(filename);  break;
        case ASSET_TYPE_MODEL:  asset->model  = load_obj(filename);    break;
        case ASSET_TYPE_SHADER: {
            for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
                const char *new_path = 0;
                if (info->file_paths[i] != 0)
                    new_path = char_array_concat(asset_folders[asset->type], info->file_paths[i]);
                asset->shader.files[i].filepath = new_path;
            }
            load_shader(&asset->shader);
        } break;         
    }

    platform_free((void *)filename);
    platform_free((void *)info->filename);
}

internal void
add_asset(void *data, void *args) {
    Assets *assets = (Assets*)data;
    Asset_Parse_Info *info = (Asset_Parse_Info*)args;

    u32 asset_array_index = info->indexes[info->type]++;
    Asset *asset = &assets->types[info->type].data[asset_array_index];
    load_asset(asset, info);
}

internal void
count_asset(void *data, void *args) {
    Assets *assets = (Assets*)data;
    Asset_Parse_Info *info = (Asset_Parse_Info*)args;

    assets->num_of_assets++;
    assets->types[info->type].num_of_assets++;
}

internal void
init_types_array(Asset *data, Asset_Array *types) {
    u32 running_total_of_assets = 0;
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) {
        types[i].data = data + (running_total_of_assets);
        running_total_of_assets += types[i].num_of_assets;
    }
}

// returns true if loading the assets fails
internal bool8
load_assets(Assets *assets, const char *filepath) {
    File file = load_file(filepath);
    if (file.size == 0) {
        logprint("load_assets()", "could not open file %s\n", filepath);
        return 1;
    }
    
    parse_asset_file(assets, &file, count_asset);
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    init_types_array(assets->data, assets->types);

    file_reset_char(&file);
    parse_asset_file(assets, &file, add_asset);
    free_file(&file);

    return 0;
}
