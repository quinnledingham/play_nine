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
parse_asset_file(Assets *assets, File *file, void (action)(void *data, void *arg, s32 asset_indices[ASSET_TYPE_AMOUNT])) {
    Asset_Parse_Info info = {};
    s32 asset_indices[ASSET_TYPE_AMOUNT] = {};
    
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
                action((void *)assets, (void *)&info, asset_indices);
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
                        action((void*)assets, (void*)&info, asset_indices);

                        // wipe out all filepaths
                        info = {};
                        info.type = ASSET_TYPE_SHADER;

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
                        logprint("parse_asset_file()", "expected ',' @ %d\n", line_num);
                        break;
                    }
                } else {
                    info.filename = token.lexeme;
                    action((void*)assets, (void*)&info, asset_indices);
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
load_asset_files(void *data, void *args, s32 asset_indices[ASSET_TYPE_AMOUNT]) {
    Assets *assets = (Assets *)data;
    Asset_Parse_Info *info = (Asset_Parse_Info *)args;

    u32 asset_array_index = asset_indices[info->type]++;   
    Asset *asset = &assets->types[info->type].data[asset_array_index];
    
    s32 asset_index = s32(asset - assets->data);
    Asset_Files *files = &assets->files[asset_index];
        
    *asset = {};
    asset->type = info->type;
    asset->tag = info->tag;
    asset->tag_length = get_length(asset->tag);
    const char *file_name = char_array_concat(asset_folders[asset->type], info->filename);

    files->num_of_files = 0;

    switch(asset->type) {
        case ASSET_TYPE_FONT:
        case ASSET_TYPE_BITMAP:
        case ASSET_TYPE_AUDIO:
        case ASSET_TYPE_MODEL:
            files->num_of_files = 1;
            files->data[0] = load_file(file_name);
        break;

        case ASSET_TYPE_SHADER: {
            for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
                files->data[i] = {};
                files->data[i].filepath = 0;
                if (info->file_paths[i] != 0) {
                    files->num_of_files++;
                    const char *path = char_array_concat(asset_folders[asset->type], info->file_paths[i]);
                    files->data[i] = load_file(path);
                }
            }
        } break;

    }

    //platform_free((void *)file_name);
    platform_free((void *)info->filename);
};

internal void
count_asset(void *data, void *args, s32 asset_indices[ASSET_TYPE_AMOUNT]) {
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
    assets->files = ARRAY_MALLOC(Asset_Files, assets->num_of_assets);
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    init_types_array(assets->data, assets->types);
    
    file_reset_char(&file);
    parse_asset_file(assets, &file, load_asset_files);

    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        Asset_Files *files = &assets->files[i];
        switch(asset->type) {
            case ASSET_TYPE_FONT: asset->font.file = files->data[0]; break;
            case ASSET_TYPE_BITMAP: asset->bitmap = load_bitmap(files->data[0], false); break;
            case ASSET_TYPE_AUDIO: asset->audio = load_ogg(files->data[0]); break;
            case ASSET_TYPE_MODEL: asset->model = load_obj(files->data[0]); break;
            case ASSET_TYPE_SHADER: {
                for (u32 shader_index = 0; shader_index < SHADER_STAGES_AMOUNT; shader_index++) {
                    asset->shader.files[shader_index].filepath = files->data[shader_index].filepath;
                }
                load_shader(&asset->shader);
                spirv_compile_shader(&asset->shader);
            } break;

        }
    }
    
    free_file(&file);
    return 0;
}

internal bool8
add_assets(Assets *assets, Asset *new_assets, u32 num_of_assets) {
    Asset *new_data = ARRAY_MALLOC(Asset, (assets->num_of_assets + num_of_assets));
    platform_memory_set(new_data, 0, sizeof(Asset) * (assets->num_of_assets + num_of_assets));
    
    Asset_Array new_types[ASSET_TYPE_AMOUNT];

    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++)
        new_types[i].num_of_assets = assets->types[i].num_of_assets;

    for (u32 i = 0; i < num_of_assets; i++) {
        Asset *asset = &new_assets[i];
        new_types[asset->type].num_of_assets++;
    }

    init_types_array(new_data, new_types);

    s32 indexes[ASSET_TYPE_AMOUNT];
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) {
        for (u32 j = 0; j < assets->types[i].num_of_assets; j++) {
            new_types[i].data[j] = assets->types[i].data[j];
        }
        indexes[i] = assets->types[i].num_of_assets;
    }

    for (u32 i = 0; i < num_of_assets; i++) {
        Asset asset = new_assets[i];
        new_types[asset.type].data[indexes[asset.type]++] = asset;
    }

    platform_free(assets->data);

    assets->num_of_assets += num_of_assets;
    assets->data = new_data;
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++)
        assets->types[i] = new_types[i];

    return 0;
}

internal u32
get_asset_size(Asset *asset) {
    u32 size = 0;

    switch(asset->type) {
        //case ASSET_TYPE_FONT:   init_font(&asset->font);               break;
        case ASSET_TYPE_BITMAP: {
            size += asset->bitmap.width * asset->bitmap.height * asset->bitmap.channels;
        } break;
        //case ASSET_TYPE_SHADER: clean_shader(&asset->shader); break;

        case ASSET_TYPE_MODEL: {
            size += asset->model.meshes_count * sizeof(Mesh);
            for (u32 i = 0; i < asset->model.meshes_count; i++) {
                size += asset->model.meshes[i].vertex_info.size * asset->model.meshes[i].vertices_count;
                size += sizeof(u32) * asset->model.meshes[i].indices_count;
            }

        } break;
    }

    return size;
}

internal void
print_assets(Assets *assets) {
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        u32 size = get_asset_size(&assets->data[i]);
        print("type: %d, tag: %s, size: %d\n", assets->data[i].type, assets->data[i].tag, size);
    }
}

//
// Save
//

internal void
save_bitmap_memory(Bitmap bitmap, FILE *file)
{
    fwrite(bitmap.memory, bitmap.dim.x * bitmap.dim.y * bitmap.channels, 1, file);
}

internal void
load_bitmap_memory(Bitmap *bitmap, FILE *file)
{
    u32 size = bitmap->dim.x * bitmap->dim.y * bitmap->channels;
    bitmap->memory = (u8*)platform_malloc(size);
    fread(bitmap->memory, size, 1, file);
}

internal void
save_mesh(Mesh mesh, FILE *file)
{
    fwrite((void*)&mesh, sizeof(Mesh), 1, file);
    fwrite(mesh.vertices, mesh.vertex_info.size, mesh.vertices_count, file);
    fwrite(mesh.indices, sizeof(u32), mesh.indices_count, file);

    if (mesh.material.diffuse_map.channels != 0) save_bitmap_memory(mesh.material.diffuse_map, file);
}

internal Mesh
load_mesh(FILE *file)
{
    Mesh mesh = {};

    fread((void*)&mesh, sizeof(Mesh), 1, file);
    mesh.vertices = platform_malloc(mesh.vertex_info.size * mesh.vertices_count);
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
    fread(mesh.vertices, mesh.vertex_info.size, mesh.vertices_count, file);
    fread(mesh.indices, sizeof(u32), mesh.indices_count, file);

    if (mesh.material.diffuse_map.channels != 0) load_bitmap_memory(&mesh.material.diffuse_map, file);

    return mesh;
}

internal void
save_assets(Assets *assets, FILE *file)
{
    //FILE *file = fopen(filename, "wb");
    //fseek(file, offset, SEEK_SET);
    fwrite(assets, sizeof(Assets), 1, file);
    fwrite(assets->data, sizeof(Asset), assets->num_of_assets, file);
    
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        Asset_Files *files = &assets->files[i];
        fwrite(asset->tag, asset->tag_length + 1, 1, file);

        switch(asset->type) {
            case ASSET_TYPE_FONT: fwrite(asset->font.file.memory, asset->font.file.size, 1, file); break;
            case ASSET_TYPE_BITMAP: save_bitmap_memory(asset->bitmap, file); break;
            
            case ASSET_TYPE_SHADER: {
                for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
                    if (asset->shader.spirv_files[i].size) {
                        fwrite(asset->shader.spirv_files[i].memory, asset->shader.spirv_files[i].size, 1, file);
                    }
                }
            } break;
            
            case ASSET_TYPE_AUDIO: fwrite(asset->audio.buffer, asset->audio.length, 1, file); break;

            case ASSET_TYPE_MODEL:{
                for (u32 i = 0; i < asset->model.meshes_count; i++) 
                    save_mesh(asset->model.meshes[i], file);
            } break;
        }
    }
    
    //fclose(file);
}

//
// Load Saved
//

internal u32
load_saved_assets(Assets *assets, const char *filename, u32 offset) // returns 0 on success
{
    FILE *file = fopen(filename, "rb");
    if (file == 0) { 
        logprint("load_saved_assets()", "could not open file %s\n", filename); 
        return 1; 
    }
    fseek(file, offset, SEEK_SET);
    fread(assets, sizeof(Assets), 1, file);
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    fread(assets->data, sizeof(Asset), assets->num_of_assets, file);

    init_types_array(assets->data, assets->types);
    
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];

        asset->tag = (const char*)platform_malloc(asset->tag_length + 1);
        fread((void*)asset->tag, asset->tag_length + 1, 1, file);
        
        switch(asset->type) {
            case ASSET_TYPE_FONT: {
                asset->font.file.memory = platform_malloc(asset->font.file.size);
                fread(asset->font.file.memory, asset->font.file.size, 1, file);
            } break;
            case ASSET_TYPE_BITMAP: {
                load_bitmap_memory(&asset->bitmap, file);
            } break;

            case ASSET_TYPE_SHADER: {
                for (u32 i = 0; i < SHADER_STAGES_AMOUNT; i++) {
                    asset->shader.files[i] = {};

                    if (asset->shader.spirv_files[i].size) {
                        asset->shader.spirv_files[i].memory = platform_malloc(asset->shader.spirv_files[i].size);
                        fread((void*)asset->shader.spirv_files[i].memory, asset->shader.spirv_files[i].size, 1, file);
                    }
                    asset->shader.spirv_files[i].path = 0;
                }

                asset->shader.compiled = false;
            } break;
            
            case ASSET_TYPE_AUDIO: {
                asset->audio.buffer = (u8*)platform_malloc(asset->audio.length);
                fread(asset->audio.buffer, asset->audio.length, 1, file);
            } break;

            case ASSET_TYPE_MODEL: {
                asset->model.meshes = (Mesh*)platform_malloc(asset->model.meshes_count * sizeof(Mesh));

                for (u32 i = 0; i < asset->model.meshes_count; i++) { 
                    asset->model.meshes[i] = load_mesh(file);
                }
            } break;
        }
    }
    
    fclose(file);
    
    return 0;
}

//
// Init
//

internal void
init_assets(Assets *assets) {
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        switch(asset->type) {
            case ASSET_TYPE_FONT:   init_font(&asset->font, asset->font.file);  break;
            case ASSET_TYPE_BITMAP: {
                render_create_texture(&asset->bitmap, TEXTURE_PARAMETERS_CHAR);
            } break;
            case ASSET_TYPE_SHADER: render_compile_shader(&asset->shader); break;
            case ASSET_TYPE_AUDIO:                                         break;
            case ASSET_TYPE_MODEL:  init_model(&asset->model);             break;
        }
    }
}

internal void
clean_assets(Assets *assets) {
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        switch(asset->type) {
            //case ASSET_TYPE_FONT:   init_font(&asset->font);               break;
            case ASSET_TYPE_BITMAP: free_bitmap(asset->bitmap); break;
            case ASSET_TYPE_SHADER: clean_shader(&asset->shader); break;
            //case ASSET_TYPE_AUDIO:                                         break;
            //case ASSET_TYPE_MODEL:  clean_model(&asset->model);             break;
        }
    }
}


internal u32
get_assets_size(Assets *assets) {
    u32 size = 0;

    size += sizeof(Assets);
    size += sizeof(Asset) * assets->num_of_assets;

    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        switch(asset->type) {
            //case ASSET_TYPE_FONT:   init_font(&asset->font);               break;
            case ASSET_TYPE_BITMAP: {
                size += asset->bitmap.width * asset->bitmap.height * asset->bitmap.channels;
            } break;
            //case ASSET_TYPE_SHADER: clean_shader(&asset->shader); break;

            case ASSET_TYPE_MODEL: {
                size += asset->model.meshes_count * sizeof(Mesh);
                for (u32 i = 0; i < asset->model.meshes_count; i++) {
                    size += asset->model.meshes[i].vertex_info.size * asset->model.meshes[i].vertices_count;
                    size += sizeof(u32) * asset->model.meshes[i].indices_count;
                }

            } break;
        }
    }

    return size;
}
