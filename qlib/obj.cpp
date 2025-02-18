#include "mtl.cpp"

enum OBJ_Token_Type {
    OBJ_TOKEN_ID,
    OBJ_TOKEN_NUMBER,
    
    OBJ_TOKEN_VERTEX,
    OBJ_TOKEN_NORMAL,
    OBJ_TOKEN_TEXTURE_COORD,
    OBJ_TOKEN_FACE,
    OBJ_TOKEN_LINE,
    OBJ_TOKEN_USEMTL,
    OBJ_TOKEN_MTLLIB,
    OBJ_TOKEN_SMOOTH_SHADING,
    
    OBJ_TOKEN_ERROR,
    OBJ_TOKEN_EOF
};

// returned from the obj scanner
struct OBJ_Token {
    s32 type;
    const char *lexeme;
    s32 ch; // char when the token is created. Used for debugging
};

union Face_Vertex {
    struct {
        u32 position_index;
        u32 uv_index;
        u32 normal_index;
    };
    u32 E[3];
};

// stores information taken from a .obj file
struct OBJ {
    u32 vertices_count;
    u32 uvs_count;
    u32 normals_count;
    u32 faces_count;
    u32 lines_count;
    u32 meshes_count;
    
    Vector3 *vertices;
    Vector2 *uvs;
    Vector3 *normals;
    Face_Vertex *face_vertices; // array of all the faces vertexs
    u32 *meshes_face_count; // the amount of faces in each mesh
    
    const char *material_filename;
};

s32 obj_valid_chars[5] = { '-', '.', '_', ':', '\\' };

internal bool8
is_valid_char(s32 ch) {
    for (s32 i = 0; i < ARRAY_COUNT(obj_valid_chars); i++) {
        if (ch == obj_valid_chars[i]) return true;
    }
    return false;
}

internal bool8
scan_is_string(File *file, s32 *ch, const char *str, u32 str_length) {
    bool8 is_string = true;
    for (u32 ch_index = 0; ch_index < str_length; ch_index++) {
        if (*ch != str[ch_index]) is_string = false;
        *ch = file_get_char(file);
    }
    
    return is_string;
}

// strings = true if you want it to return filenames and numbers
// doesn't malloc any memory unless strings = true
// buffer max length = 40
internal OBJ_Token
scan_obj(File *file, s32 *line_num, bool8 strings, char *buffer) {
    X:
    s32 ch;
    while((ch = file_get_char(file)) != EOF && (ch == 9 || ch == 13 || ch == ' ' || ch == '/' || ch == '\n')); // remove tabs
    
    switch(ch) {
        case EOF: { return { OBJ_TOKEN_EOF, 0, ch }; } break;
        
        /*
        case '\n':
        {
            (*line_num)++;
            goto X;
        } break;
        */

        case '#': {
            while((ch = file_get_char(file)) != EOF && (ch != '\n'));
            file_un_char(file);
            goto X;
        } break;
        
        default: {
            s32 last_ch = file_previous_char(file);

            // only after a newline
            if (isalpha(ch) && last_ch == '\n') {
                s32 next_ch = file_peek_char(file);

                if (ch == 'v') {
                    ch = file_get_char(file);
                    next_ch = file_peek_char(file);

                    if      (ch == 'n' && next_ch == ' ') return { OBJ_TOKEN_NORMAL, "vn", ch };
                    else if (ch == 't' && next_ch == ' ') return { OBJ_TOKEN_TEXTURE_COORD, "vt", ch };
                    else if (ch == ' ') return { OBJ_TOKEN_VERTEX, "v", ch };
                }
                else if (ch == 'f' && next_ch == ' ') return { OBJ_TOKEN_FACE, "f", ch };
                else if (ch == 'l' && next_ch == ' ') return { OBJ_TOKEN_LINE, "l", ch };
                else if (ch == 'm') { if (scan_is_string(file, &ch, "mtllib", 6)) return { OBJ_TOKEN_MTLLIB, "mtllib", ch }; }
                else if (ch == 'u') { if (scan_is_string(file, &ch, "usemtl", 6)) return { OBJ_TOKEN_USEMTL, "usemtl", ch }; }
                else if (ch == 's') return { OBJ_TOKEN_SMOOTH_SHADING, "s", ch };
            }
            
            if (strings) {
                int length = 0;
                buffer[length] = ch;
                do {
                    if (length >= 40) return { OBJ_TOKEN_ERROR, "error", ch };
                    buffer[length++] = ch;
                    ch = file_get_char(file);
                } while((isalpha(ch) || isdigit(ch) || is_valid_char(ch)) && ch != ' ' && ch != '/' && ch != EOF);
                buffer[length] = 0;
                return { OBJ_TOKEN_ID, buffer, ch };
            }
            
            goto X;
        } break;
    }
    
    return { OBJ_TOKEN_ERROR, "error", ch };
}

internal const char*
skip_whitespace(const char *ptr) {
    X:
    while(*ptr != EOF && (*ptr == 9 || *ptr == 13 || *ptr == ' ' || *ptr == '/' || *ptr == '\n')) {
        ptr++;
    }

    if (*ptr == '#') {
        while(*ptr != EOF && (*ptr != '\n')) { ptr++; }
        goto X;
    }

    return ptr;
}

internal void
parse_v3_obj(File *file, s32 *line_num, Vector3 *v) {   
    for (u32 i = 0; i < 3; i++) {
        file->ch = (char*)skip_whitespace(file->ch);
        file->ch = (char*)char_array_to_float32(file->ch, &v->E[i]);
    }
    file->ch = (char*)skip_whitespace(file->ch);
}

internal void
parse_face_vertex(File *file, s32 *line_num, Face_Vertex *f) {
    for (u32 i = 0; i < 3; i++) {
        file->ch = (char*)skip_whitespace(file->ch);
        file->ch = (char*)char_array_to_u32(file->ch, &f->E[i]);
    }
}

internal void
obj_count(OBJ *obj, File file) {
    char buffer[40];
    OBJ_Token token = {};
    s32 line_num = 1;
    do {
        token = scan_obj(&file, &line_num, false, buffer);
        
        switch(token.type) {
            case OBJ_TOKEN_VERTEX:        obj->vertices_count++; break;
            case OBJ_TOKEN_NORMAL:        obj->normals_count++;  break;
            case OBJ_TOKEN_TEXTURE_COORD: obj->uvs_count++;      break;
            case OBJ_TOKEN_FACE:          obj->faces_count++;    break;
            case OBJ_TOKEN_LINE:          obj->lines_count++;    break;
            case OBJ_TOKEN_USEMTL:        obj->meshes_count++;   break;
        }
        
    } while(token.type != OBJ_TOKEN_EOF);
}

internal void
obj_fill_arrays(OBJ *obj, File file, Model *model) {
    char buffer[40];
    OBJ_Token last_token = {};
    OBJ_Token token = {};
    s32 line_num = 1;
    
    s32 meshes_index        = -1;
    u32 vertices_index      = 0;
    u32 uvs_index           = 0;
    u32 normals_index       = 0;
    u32 face_vertices_index = 0;
    /*
    while(*file.ch != EOF)
    {
        file.ch = (char*)skip_whitespace(file.ch);
        if (*file.ch == 'v')
        {
            file.ch++;
            if (*file.ch == ' ' || *file.ch == '\t')
            {
                file.ch++;
                parse_v3_obj(&file, &line_num, &obj->vertices[vertices_index++]);
            }
            else if (*file.ch == 'n') 
            {
                file.ch++;
                parse_v3_obj(&file, &line_num, &obj->normals[normals_index++]);
            }
            else if (*file.ch == 't')
            {
                file.ch++;
                file.ch = (char*)parse_float(file.ch, &obj->uvs[uvs_index].x);
                file.ch = (char*)parse_float(file.ch, &obj->uvs[uvs_index].y);
                uvs_index++;
            }
        }
        else if (*file.ch == 'f')
        {
            file.ch++;
            if (*file.ch == ' ' || *file.ch == '\t')
            {
                file.ch++;
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                obj->meshes_face_count[meshes_index]++;
            }
        }
        else if (*file.ch == 'l')
        {
            scan_obj(&file, &line_num, true, buffer);
            scan_obj(&file, &line_num, true, buffer);
        }
        else if (*file.ch == 'm')
        {
            file.ch++;
            if (file.ch[0] == 't' && file.ch[1] == 'l' && file.ch[2] == 'l' && 
                file.ch[3] == 'i' && file.ch[4] == 'b' && file.ch[5] == ' ')
            {
                file.ch += 5;

                token = scan_obj(&file, &line_num, true, buffer);
                obj->material_filename = string_malloc(token.lexeme);
            }
        }
        else if (*file.ch == 'u')
        {
            file.ch++;
            if (file.ch[0] == 's' && file.ch[1] == 'e' && file.ch[2] == 'm' && 
                file.ch[3] == 't' && file.ch[4] == 'l' && file.ch[5] == ' ')
            {
                file.ch += 5;

                meshes_index++;
                token = scan_obj(&file, &line_num, true, buffer);
                model.meshes[meshes_index].material.id = string_malloc(token.lexeme);
            }
        }

        while(*file.ch != EOF && *file.ch != '\n')
        {
            file.ch++;
        }
    }
    */
    
    do
    {
        last_token = token;
        token = scan_obj(&file, &line_num, false, buffer);
        
        if      (token.type == OBJ_TOKEN_VERTEX) parse_v3_obj(&file, &line_num, &obj->vertices[vertices_index++]);
        else if (token.type == OBJ_TOKEN_NORMAL) parse_v3_obj(&file, &line_num, &obj->normals[normals_index++]);
        else if (token.type == OBJ_TOKEN_TEXTURE_COORD)
        {
            file.ch = (char*)skip_whitespace(file.ch);
            file.ch = (char*)char_array_to_float32(file.ch, &obj->uvs[uvs_index].x);
            file.ch = (char*)skip_whitespace(file.ch);
            file.ch = (char*)char_array_to_float32(file.ch, &obj->uvs[uvs_index].y);
            file.ch = (char*)skip_whitespace(file.ch);
            uvs_index++;
        }
        else if (token.type == OBJ_TOKEN_FACE)
        {
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            
            obj->meshes_face_count[meshes_index]++;
        }
        else if (token.type == OBJ_TOKEN_LINE)
        {
            token = scan_obj(&file, &line_num, true, buffer);
            token = scan_obj(&file, &line_num, true, buffer);
        }
        else if (token.type == OBJ_TOKEN_MTLLIB) // get the material file to load
        {
            token = scan_obj(&file, &line_num, true, buffer);
            obj->material_filename = string_malloc(token.lexeme);
        }
        else if (token.type == OBJ_TOKEN_USEMTL) // different mesh every time the material is changed
        {
            meshes_index++;
            token = scan_obj(&file, &line_num, true, buffer);
            model->meshes[meshes_index].material.id = string_malloc(token.lexeme);
        }
    } while(token.type != OBJ_TOKEN_EOF);
}

internal void
model_create_meshes(Model *model, OBJ obj, MTL mtl) {
    s32 lower_range = 0;
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
        Mesh *mesh = &model->meshes[mesh_index];        
        mesh->vertex_info = get_vertex_xnu_info();
        s32 mesh_face_vertices_count = obj.meshes_face_count[mesh_index] * 3;
        
        mesh->indices_count = mesh_face_vertices_count;
        mesh->indices = ARRAY_MALLOC(u32, mesh->indices_count);
        
        mesh->vertices_count = mesh_face_vertices_count;
        mesh->vertices = ARRAY_MALLOC(Vertex_XNU, mesh->vertices_count);
        
        // put unique face vertices in mesh vertices array
        u32 vertices_index = 0;
        u32 indices_index = 0;
        for (s32 i = lower_range; i < mesh_face_vertices_count + lower_range; i++) {
            mesh->indices[indices_index++] = vertices_index;
            
            Vertex_XNU *vertices = (Vertex_XNU *)mesh->vertices;
            Vertex_XNU *vertex = &vertices[vertices_index++];
            vertex->position   = obj.vertices[obj.face_vertices[i].position_index - 1];
            vertex->normal     = obj.normals [obj.face_vertices[i].normal_index   - 1];
            vertex->uv         = obj.uvs     [obj.face_vertices[i].uv_index       - 1];
        }
        
        // assign material aka load material
        for (u32 i = 0; i < mtl.materials_count; i++) {
            if (equal(mesh->material.id, mtl.materials[i].id)) 
                mesh->material = mtl.materials[i];
        }
                    
        lower_range += mesh_face_vertices_count;
    }
}

Model load_obj(File file) {
    Model model = {};
    OBJ obj = {};
    
    if (!file.size) { 
        logprint("load_obj()", "could not read object file\n"); 
        return model; 
    }

    // count components
    obj_count(&obj, file);
    
    // allocate space for the components
    obj.vertices          = ARRAY_MALLOC(Vector3, obj.vertices_count);
    obj.uvs               = ARRAY_MALLOC(Vector2, obj.uvs_count);
    obj.normals           = ARRAY_MALLOC(Vector3, obj.normals_count);
    obj.face_vertices     = ARRAY_MALLOC(Face_Vertex, obj.faces_count * 3);
    obj.meshes_face_count = ARRAY_MALLOC(u32, obj.meshes_count);
    memset(obj.meshes_face_count, 0, sizeof(u32) * obj.meshes_count); // ++ to count so need it to start at zero
    
    model.meshes_count = obj.meshes_count;
    model.meshes = ARRAY_MALLOC(Mesh, model.meshes_count);
    
    file_reset_char(&file);
    
    obj_fill_arrays(&obj, file, &model);
        
    const char *filepath = get_path(file.filepath);
    MTL mtl = load_mtl(filepath, obj.material_filename);
    platform_free((void*)filepath);
    
    // creating the model's meshes
    model_create_meshes(&model, obj, mtl);
    
    platform_free(mtl.materials);
    
    platform_free(obj.vertices);
    platform_free(obj.uvs);
    platform_free(obj.normals);
    platform_free(obj.face_vertices);
    platform_free(obj.meshes_face_count);
    
    return model;
}
