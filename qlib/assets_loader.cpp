internal void
load_asset_file(Asset *asset, u32 type, Asset_Decl *decl) {
    asset->type = type;
    asset->id = decl->id;
    asset->files_count = 0;
    
    File *files = asset->files;
    switch(asset->type) {
        case ASSET_TYPE_FONT:
        case ASSET_TYPE_BITMAP:
        case ASSET_TYPE_AUDIO:
        case ASSET_TYPE_MODEL: {
            asset->files_count = 1;
            const char *file_name = char_array_concat(asset_folders[asset->type], decl->filename);
            files[0] = load_file(file_name);
        } break;

        case ASSET_TYPE_SHADER: {
            u32 i = 0;
            for (u32 shader_index = 0; shader_index < SHADER_STAGES_AMOUNT; shader_index++) {
                if (shader_index == SHADER_STAGE_COMPUTE && asset->files_count != 0)
                    break;

                if (shader_index == SHADER_STAGE_COMPUTE)
                    i = 0;
                
                files[i] = {};
                files[i].filepath = 0;
               
                if (decl->E[i] != 0) {
                    asset->files_count++;
                    const char *path = char_array_concat(asset_folders[asset->type], decl->E[i]);
                    files[i] = load_file(path);
                }
                i++;
            }
        } break;

        case ASSET_TYPE_ATLAS: {
            asset->files_count = 2;
            const char *bitmap_file_path = char_array_concat(asset_folders[asset->type], decl->E[0]);
            const char *tex_coord_file_path = char_array_concat(asset_folders[asset->type], decl->E[1]);
            files[0] = load_file(bitmap_file_path);
            files[1] = load_file(tex_coord_file_path);
        } break;

    }
}

internal void
init_types_array(Asset *data, Asset_Array *types) {
    u32 running_total_of_assets = 0;
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) {
        types[i].data = data + (running_total_of_assets);
        running_total_of_assets += types[i].num_of_assets;
    }
}

internal bool8
load_asset_files(Assets *assets, Asset_Decl *decls, u32 decls_count) {

    assets->num_of_assets = 0;

    // count assets
    u32 current_type = 0;
    for (u32 i = 0; i < decls_count; i++) {
        if (decls[i].id < ASSET_TYPE_AMOUNT) {
            current_type = decls[i].id;
        } else {
            assets->types[current_type].num_of_assets++;
            assets->num_of_assets++;
        }
    }
        
    assets->data = ARRAY_MALLOC_CLEAR(Asset, assets->num_of_assets);
    init_types_array(assets->data, assets->types);

    s32 asset_indices[ASSET_TYPE_AMOUNT] = {};

    current_type = 0;
    u32 asset_index = 0;
    for (u32 i = 0; i < decls_count; i++) {        
        if (decls[i].id < ASSET_TYPE_AMOUNT) {
            current_type = decls[i].id;
        } else {
            u32 asset_array_index = asset_indices[current_type]++;   
            Asset *asset = &assets->types[current_type].data[asset_array_index];

            load_asset_file(asset, current_type, &decls[i]);
        }
    }
    
    return 0;
}

// returns true if loading the assets fails
// loads assets from the individual files specified in load asset files
internal bool8
load_assets_from_files(Assets *assets) {
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        File *files = asset->files;
        switch(asset->type) {
            case ASSET_TYPE_FONT: asset->font.file = files[0]; break;
            case ASSET_TYPE_BITMAP: asset->bitmap = load_bitmap(files[0], false); break;
            case ASSET_TYPE_AUDIO: asset->audio = load_ogg(files[0]); break;
            case ASSET_TYPE_MODEL: asset->model = load_obj(files[0]); break;
            case ASSET_TYPE_SHADER: {
                // compile compute shader
                if (asset->files_count == 1) {
                    asset->shader.files[SHADER_STAGE_COMPUTE] = files[0];
                    spirv_compile_shader(&asset->shader);
                    break;
                }

                // all other shaders
                for (u32 shader_index = 0; shader_index < SHADER_STAGES_AMOUNT - 1; shader_index++) {
                    asset->shader.files[shader_index] = files[shader_index];
                }
                spirv_compile_shader(&asset->shader);
            } break;
            case ASSET_TYPE_ATLAS: {
                asset->atlas.bitmap = load_bitmap(files[0], false);
                platform_memory_copy(asset->atlas.texture_coords, files[1].memory, sizeof(Texture_Coords) * asset->atlas.max_textures);
            } break;
        }
    }
    
    return 0;
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
save_asset_files(Assets *assets, const char *file_path) {
    FILE *file = fopen(file_path, "wb");
    
    fwrite(assets, sizeof(Assets), 1, file);
    fwrite(assets->data, sizeof(Asset), assets->num_of_assets, file);
    
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        File *files = asset->files;
                
        for (u32 file_index = 0; file_index < ARRAY_COUNT(files); file_index++) {
            File *asset_file = &files[file_index];
            if (asset_file->size != 0) {
                fwrite(asset_file->memory, asset_file->size, 1, file);
                if (asset_file->filepath_length != 0)
                    fwrite(asset_file->filepath, asset_file->filepath_length + 1, 1, file);
            }
        }
    }

    fclose(file);
}

internal void
save_assets(Assets *assets, const char *file_path) {
    FILE *file = fopen(file_path, "wb");

    fwrite(assets, sizeof(Assets), 1, file);
    fwrite(assets->data, sizeof(Asset), assets->num_of_assets, file);
    
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];

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

            case ASSET_TYPE_MODEL: {
                for (u32 i = 0; i < asset->model.meshes_count; i++) 
                    save_mesh(asset->model.meshes[i], file);
            } break;

            case ASSET_TYPE_ATLAS: {
                save_bitmap_memory(asset->atlas.bitmap, file);
                fwrite(asset->atlas.texture_coords, sizeof(Texture_Coords) * asset->atlas.max_textures, 1, file);
            } break;
        }
    }
    
    fclose(file);
}

//
// Load Saved
//

internal u32
load_saved_asset_files(Assets *assets, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file == 0) {
        logprint("load_saved_asset_files()", "could not open file %s\n", file_path); 
        return 1; 
    }
    
    fread(assets, sizeof(Assets), 1, file);
    assets->data = ARRAY_MALLOC_CLEAR(Asset, assets->num_of_assets);
    fread(assets->data, sizeof(Asset), assets->num_of_assets, file);
    
    init_types_array(assets->data, assets->types);
    
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        File *files = asset->files;
        
        for (u32 file_index = 0; file_index < ARRAY_COUNT(files); file_index++) {
            File *asset_file = &files[file_index];
            if (asset_file->size != 0) {
            
                asset_file->memory = platform_malloc(asset_file->size);
                asset_file->filepath = (const char *)platform_malloc(asset_file->filepath_length + 1);
                fread(asset_file->memory, asset_file->size, 1, file);
                if (asset_file->filepath_length != 0)
                    fread((void*)asset_file->filepath, asset_file->filepath_length + 1, 1, file);
                asset_file->ch = (char*)asset_file->memory;
            }
        }
    }

    fclose(file);

    return 0;
}

// returns 0 on success 
internal u32
load_saved_assets(Assets *assets, const char *filename, u32 offset) {
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

            case ASSET_TYPE_ATLAS: {
                load_bitmap_memory(&asset->atlas.bitmap, file);
                fread((void *)asset->atlas.texture_coords, sizeof(Texture_Coords) * asset->atlas.max_textures, 1, file);
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
                gfx.create_texture(&asset->bitmap, TEXTURE_PARAMETERS_CHAR);
            } break;
            //case ASSET_TYPE_SHADER: render_compile_shader(&asset->shader); break;
            case ASSET_TYPE_AUDIO:                                         break;
            case ASSET_TYPE_MODEL:  init_model(&asset->model);             break;
            case ASSET_TYPE_ATLAS: texture_atlas_init(&asset->atlas, 3); break;
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

internal bool8
load_assets(Assets *assets, Asset_Decl *decls, u32 decls_count) {
    if (assets->loaded) {
        logprint("load_assets()", "already loaded\n");
        return 1;
    }
    
    bool8 load_failed = false;
    const char *files_save_filepath = "files.save";
    const char *assets_save_filepath = "assets.save";
    u32 offset = 0;

    if (load_saved_assets(assets, assets_save_filepath, 0)) {
        if (load_saved_asset_files(assets, files_save_filepath)) {
            load_asset_files(assets, decls, decls_count);
            save_asset_files(assets, files_save_filepath);
        }
        load_assets_from_files(assets); 
        save_assets(assets, assets_save_filepath);
    }

    init_assets(assets);
    
    assets->loaded = true;
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

internal void
print_assets(Assets *assets) {
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        u32 size = get_asset_size(&assets->data[i]);
        print("type: %d, id: %s, size: %d\n", assets->data[i].type, assets->data[i].id, size);
    }
}
