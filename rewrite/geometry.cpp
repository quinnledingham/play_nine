struct Obj_Token {
  s32 type;
  char lexeme[20];

  void print() {
    log("TOKEN: %d, %s\n", type, lexeme);
  }
};

union Obj_Face_Vertex {
    struct {
        u32 position_index;
        u32 uv_index;
        u32 normal_index;
    }; 
    u32 E[3];
};

struct Obj_File {
  // Scanning
  Obj_Token token;
  s32 line_num;
  bool8 skip_floats;

  u32 vertices_count;
  u32 uvs_count;
  u32 normals_count;
  u32 faces_count;
  u32 lines_count;
  u32 meshes_count;

  Vector3 *vertices;
  Vector2 *uvs;
  Vector3 *normals;
  Obj_Face_Vertex *face_vertices; // array of all the faces vertexs
  u32 *meshes_face_count; // the amount of faces in each mesh
};

enum Obj_Token_Type {
    OBJ_TOKEN_ID,
    OBJ_TOKEN_NUMBER,
    
    OBJ_TOKEN_VERTEX,
    OBJ_TOKEN_NORMAL,
    OBJ_TOKEN_TEXTURE_COORD,
    OBJ_TOKEN_PARAMETER_SPACE_VERTEX,
    OBJ_TOKEN_FACE,
    OBJ_TOKEN_LINE,
    OBJ_TOKEN_USEMTL,
    OBJ_TOKEN_MTLLIB,
    OBJ_TOKEN_SMOOTH_SHADING,
    OBJ_TOKEN_OBJECT,
    
    OBJ_TOKEN_ERROR,
    OBJ_TOKEN_EOF
};

internal void
obj_scan_error(Obj_File *obj, const char *msg) {
  log_error("obj_scan(): %s @ %d\n", msg, obj->line_num);
  obj->token.type = OBJ_TOKEN_ERROR;
}

internal char*
file_get_char(char *ptr, s32 *ch) {
  *ch = *ptr;
  return ptr + 1;
}

internal char*
file_equal(char *ptr, const char* str, bool8 *equal) {
  char *str_ptr = (char *)str;

  while(*str_ptr != 0) {
    if (*ptr != *str_ptr) {
      *equal = false;
      return ptr;
    }
    ptr++;
    str_ptr++;
  }
  *equal = true;
  return ptr;
}

internal char*
file_confirm(char *ptr, s32 ch, bool8 *result) {
  if (*ptr != ch) {
    *result = false;
  }

  *result = true;
  return ptr + 1;
}

// sets token to error if space is not next;
internal char*
space_next(char *ptr, Obj_File *obj) {
  bool8 space = false;
  ptr = file_confirm(ptr, ' ', &space);
  if (!space) {
    obj_scan_error(obj, "expected space");
  }
  return ptr;
}

internal char*
file_get_string(char *ptr, char *buffer) {
  while(*ptr != EOF && *ptr != '\n' && *ptr != '\r' && *ptr != ' ') {
    *buffer = *ptr;
    *buffer++;

    ptr++;
  }

  return ptr;
}

internal char*
skip_line(char *ptr, Obj_File *obj) {
  while(*ptr != EOF && *ptr != '\n') {
    ptr++;
  }
  //(obj->line_num)++;
  return ptr;
}

// usemtl skibidi
internal char*
scan_space_string(char *ptr, Obj_File *obj, const char *expected, s32 type) {
  bool8 is_expected = false;
  ptr = file_equal(ptr, expected, &is_expected);
  if (is_expected) {
    bool8 space = false;
    ptr = file_confirm(ptr, ' ', &space);
    if (!space) {
      obj_scan_error(obj, "expected space");
    } else {
      obj->token.type = type;
      ptr = file_get_string(ptr, obj->token.lexeme);
    }
  }
  return ptr;
}

internal char*
obj_scan(Obj_File *obj, File *file, char *ptr) {
  X:

  s32 ch;
  ptr = file_get_char(ptr, &ch);
  while(ch != EOF && (ch == 9 || ch == 13 || ch == ' ' || ch == '\r')) { // remove tabs
    ptr = file_get_char(ptr, &ch);
  }

  switch(ch) {
    case 0: // EOF in File struct
    case EOF:
      obj->token.type = OBJ_TOKEN_EOF;
      break;

    case '\n':
      (obj->line_num)++;
      goto X;
      break;

    case '#':
      /*
      while(ch != EOF && ch != '\n') {
        ptr = file_get_char(ptr, &ch);
      }
      (obj->line_num)++;
      */
      ptr = skip_line(ptr, obj);
      goto X;

    case 'm':
      ptr = scan_space_string(ptr, obj, "tllib", OBJ_TOKEN_MTLLIB);
      break;
    case 'u':
      ptr = scan_space_string(ptr, obj, "semtl", OBJ_TOKEN_USEMTL);
      break;

    // Object groups
    case 'o': {
      bool8 space = false;
      ptr = file_confirm(ptr, ' ', &space);
      if (!space) {
        obj_scan_error(obj, "expected space");
      } else {
        obj->token.type = OBJ_TOKEN_OBJECT;
        ptr = file_get_string(ptr, obj->token.lexeme);
      }
    } break;

    // Vertices
    case 'v': {
      ch = *ptr++;
      switch(ch) {
        case ' ': obj->token.type = OBJ_TOKEN_VERTEX;                 break;
        case 't': obj->token.type = OBJ_TOKEN_TEXTURE_COORD;          break;
        case 'n': obj->token.type = OBJ_TOKEN_NORMAL;                 break;
        case 'p': obj->token.type = OBJ_TOKEN_PARAMETER_SPACE_VERTEX; break;
        default: {
          obj_scan_error(obj, "not a valid type");
        } break;
      }

      if (obj->token.type != OBJ_TOKEN_VERTEX)
        ptr = space_next(ptr, obj);
      if (obj->skip_floats)
        ptr = skip_line(ptr, obj);
    } break;

    // Faces
    case 'f': {
      obj->token.type = OBJ_TOKEN_FACE;
      ptr = space_next(ptr, obj);
      if (obj->skip_floats)
        ptr = skip_line(ptr, obj);
    } break;

    // Line
    case 'l': {
      obj->token.type = OBJ_TOKEN_LINE;
      ptr = space_next(ptr, obj);
    } break;

    case 's': {
      obj->token.type = OBJ_TOKEN_SMOOTH_SHADING;
      ptr = space_next(ptr, obj);
      if (obj->token.type != OBJ_TOKEN_ERROR)
        ptr = file_get_string(ptr, obj->token.lexeme);
    } break;

    default:
      obj->token.type = OBJ_TOKEN_ERROR;
      break;
  }

  return ptr;
}

internal void
obj_count(Obj_File *obj, File file) {
  obj->line_num = 1;
  obj->skip_floats = true;
  char *ptr = (char *)file.memory;
  do {
      ptr = obj_scan(obj, &file, ptr);
      switch(obj->token.type) {
        case OBJ_TOKEN_EOF:
          print("EOF\n");
          break;
        case OBJ_TOKEN_ERROR:
          obj->token.print();
          print("OBJ ERROR @ %d\n", obj->line_num);
          break;

        case OBJ_TOKEN_VERTEX:        obj->vertices_count++; break;
        case OBJ_TOKEN_NORMAL:        obj->normals_count++;  break;
        case OBJ_TOKEN_TEXTURE_COORD: obj->uvs_count++;      break;
        case OBJ_TOKEN_FACE:          obj->faces_count++;    break;
        case OBJ_TOKEN_LINE:          obj->lines_count++;    break;
        case OBJ_TOKEN_USEMTL:        obj->meshes_count++;   break;
      }
  } while(ptr - (char*)file.memory < file.size);
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

internal char*
parse_floats_obj(char *ptr, float32 *E, u32 count) {   
  for (u32 i = 0; i < count; i++) {
    ptr = (char*)skip_whitespace(ptr);
    ptr = (char*)char_array_to_float32(ptr, &E[i]);
  }
  ptr = (char*)skip_whitespace(ptr);
  return ptr;
}

internal char*
parse_face_vertex(char *ptr, Obj_Face_Vertex *f) {
  for (u32 i = 0; i < 3; i++) {
    ptr = (char*)skip_whitespace(ptr);
    ptr = (char*)char_array_to_u32(ptr, &f->E[i]);
  }
  return ptr;
}

internal void
obj_fill_arrays(Obj_File *obj, File file, Geometry *model) {
  char buffer[40];
  obj->token = {};
  obj->skip_floats = false;
  Obj_Token last_token = {};
  
  s32 meshes_index        = -1;
  u32 vertices_index      = 0;
  u32 uvs_index           = 0;
  u32 normals_index       = 0;
  u32 face_vertices_index = 0;

  obj->line_num = 1;
  char *ptr = (char *)file.memory;

  do
  {
    last_token = obj->token;
    ptr = obj_scan(obj, &file, ptr);

    switch(obj->token.type) {
      case OBJ_TOKEN_VERTEX:        ptr = parse_floats_obj(ptr, obj->vertices[vertices_index++].E, 3); break;
      case OBJ_TOKEN_NORMAL:        ptr = parse_floats_obj(ptr, obj->normals[normals_index++].E,   3); break;
      case OBJ_TOKEN_TEXTURE_COORD: ptr = parse_floats_obj(ptr, obj->uvs[uvs_index++].E,           2); break;
      case OBJ_TOKEN_FACE: {
        ptr = parse_face_vertex(ptr, &obj->face_vertices[face_vertices_index++]);
        ptr = parse_face_vertex(ptr, &obj->face_vertices[face_vertices_index++]);
        ptr = parse_face_vertex(ptr, &obj->face_vertices[face_vertices_index++]);
        
        obj->meshes_face_count[meshes_index]++;
      } break;
      case OBJ_TOKEN_USEMTL: {
        meshes_index++;
        ptr = obj_scan(obj, &file, ptr);
        obj->token.print();
      }
    }
/*
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
    */
  } while(obj->token.type != OBJ_TOKEN_EOF);
}

// stores information taken from .mtl file
struct Mtl_File {
  Material *materials; // loaded from the corresponding material file
  u32 materials_count;
};

internal void
model_create_meshes(Geometry *model, Obj_File obj, Mtl_File mtl) {
  s32 lower_range = 0;
  for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
    Mesh *mesh = &model->meshes[mesh_index];        
    mesh->vertex_info = Vertex_XNU::info();
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
    
    /*
    // assign material aka load material
    for (u32 i = 0; i < mtl.materials_count; i++) {
        if (equal(mesh->material.id, mtl.materials[i].id)) 
            mesh->material = mtl.materials[i];
    }
    */

    lower_range += mesh_face_vertices_count;
  }
}

internal Geometry
load_obj(File file) {
  Geometry geo = {};

  print("loading obj: %s\n", file.path.str());

  Obj_File obj = {};
  obj_count(&obj, file);

  // allocate space for the components
  obj.vertices          = ARRAY_MALLOC(Vector3, obj.vertices_count);
  obj.uvs               = ARRAY_MALLOC(Vector2, obj.uvs_count);
  obj.normals           = ARRAY_MALLOC(Vector3, obj.normals_count);
  obj.face_vertices     = ARRAY_MALLOC(Obj_Face_Vertex, (obj.faces_count * 3));
  obj.meshes_face_count = ARRAY_MALLOC(u32, obj.meshes_count);
  memset(obj.meshes_face_count, 0, sizeof(u32) * obj.meshes_count); // ++ to count so need it to start at zero
  
  geo.meshes_count = obj.meshes_count;
  geo.meshes = ARRAY_MALLOC(Mesh, geo.meshes_count);

  obj_fill_arrays(&obj, file, &geo);

  Mtl_File mtl = {};
  model_create_meshes(&geo, obj, mtl);

  free(obj.vertices);
  free(obj.uvs);
  free(obj.normals);
  free(obj.face_vertices);
  free(obj.meshes_face_count);

  return geo;
}

internal void
init_geometry(Geometry *geo) {
  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    vulkan_init_mesh(mesh);
  }
}

internal void
draw_geometry(Geometry *geo) {
  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    vulkan_draw_mesh(mesh);
  }
}