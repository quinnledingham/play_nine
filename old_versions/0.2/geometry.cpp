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

  u32 materials_count;
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

  *buffer = 0;

  return ptr;
}

internal char*
skip_line(char *ptr) {
  while(*ptr != EOF && *ptr != '\n') {
    ptr++;
  }
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
      ptr = skip_line(ptr);
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
        ptr = skip_line(ptr);
    } break;

    // Faces
    case 'f': {
      obj->token.type = OBJ_TOKEN_FACE;
      ptr = space_next(ptr, obj);
      if (obj->skip_floats)
        ptr = skip_line(ptr);
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
          //print("EOF\n");
          break;
        case OBJ_TOKEN_ERROR:
          obj->token.print();
          print("OBJ ERROR @ %d\n", obj->line_num);
          break;

        case OBJ_TOKEN_VERTEX:        obj->vertices_count++;  break;
        case OBJ_TOKEN_NORMAL:        obj->normals_count++;   break;
        case OBJ_TOKEN_TEXTURE_COORD: obj->uvs_count++;       break;
        case OBJ_TOKEN_FACE:          obj->faces_count++;     break;
        case OBJ_TOKEN_LINE:          obj->lines_count++;     break;
        case OBJ_TOKEN_USEMTL:        obj->meshes_count++;    break;
        case OBJ_TOKEN_MTLLIB:        obj->materials_count++; break;
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

internal char*
space_separated(char *ptr, char *buffer, u32 buffer_size) {
  X:
  char *start = ptr;
  s32 ch;
  while((ch = *ptr++) != EOF && (ch == 9 || ch == 13 || ch == ' ' || ch == '\r' || ch == '\n'));

  switch(ch) {
    case 0: // EOF in File struct
    case EOF:
      break;

    case '#':
      ptr = skip_line(ptr);
      goto X;

    default: {
      u32 size = 0;
      ptr--;
      while((ch = *ptr++) != EOF && (ch != 0 && ch != 9 && ch != 13  && ch != ' '  && ch != '\r'  && ch != '\n')) {
        if (size >= buffer_size) {
          log_error("space_separated(): exceeded buffer size\n");
          //ASSERT(0);
          break;
        }

        *buffer = ch;
        buffer++;
        size++;
      }
      *buffer = 0;
    } break;
  }

  return ptr;
}

internal void
load_mtl(Material_Library *mtllib, const char *path, const char *name) {
  mtllib->name = String(name);

  String filepath = String(path, name);
  File file = load_file(filepath.str());
  filepath.destroy();

  char *ptr; // points to next place in file
  const u32 buffer_size = 50;
  char buffer[buffer_size];

  // counting how many materials
  ptr = (char *)file.memory;
  while(in_file(&file, ptr)) {
    ptr = space_separated(ptr, buffer, buffer_size);
    if (!strcmp(buffer, "newmtl")) {
      mtllib->materials_count++;
    }
  }

  // allocating the materials
  mtllib->materials = ARRAY_MALLOC(Material, mtllib->materials_count);
  memset(mtllib->materials, 0, sizeof(Material) * mtllib->materials_count);

  // load float values
  Material *mtl = &mtllib->materials[-1];
  ptr = (char *)file.memory;
  while (in_file(&file, ptr)) {
    ptr = space_separated(ptr, buffer, buffer_size);
    if (!strcmp(buffer, "newmtl")) {
      mtl++;
      ptr = space_separated(ptr, buffer, buffer_size);
      mtl->id = String(buffer);
    } else if (!strcmp(buffer, "Ns")) {
      ptr = (char*)char_array_to_float32(ptr, &mtl->specular_exponent);
    } else if (!strcmp(buffer, "Ka")) {
      ptr = parse_floats_obj(ptr, mtl->ambient.E, 3);
    } else if (!strcmp(buffer, "Ks")) {
      ptr = parse_floats_obj(ptr, mtl->specular.E, 3);
    } else if (!strcmp(buffer, "Ke")) {
      ptr = parse_floats_obj(ptr, mtl->emissive_coefficent.E, 3);
    } else if (!strcmp(buffer, "Ni")) {
      ptr = (char*)char_array_to_float32(ptr, &mtl->optical_density);
    } else if (!strcmp(buffer, "d")) {
      ptr = (char*)char_array_to_float32(ptr, &mtl->dissolve);
    } else if (!strcmp(buffer, "Ns")) {
      ptr = (char*)char_array_to_float32(ptr, &mtl->specular_exponent);
    } else if (!strcmp(buffer, "illum")) {
      ptr = (char*)char_array_to_u32(ptr, &mtl->illumination);
    } else if (!strcmp(buffer, "map_Kd")) {

      ptr = space_separated(ptr, buffer, buffer_size);
      String bitmap_filepath = String(path, buffer);

      File bitmap_file = load_file(bitmap_filepath.str());
      mtl->diffuse_map = load_bitmap(bitmap_file);
      destroy_file(&bitmap_file);

      bitmap_filepath.destroy();
    } 

  }

  destroy_file(&file);
}

internal void
destory_mtllib(Material_Library *lib) {
  lib->name.destroy();
  free(lib->materials);
}

internal Material_Library*
find_mtllib(const char *name) {
  ASSERT(assets.mtllibs.buffer.memory != 0);

  for (u32 i = 0; i < assets.mtllibs.count; i++) {
    Material_Library *lib = find_mtllib(i);
    if (lib->name.str() == 0)
      continue;

    if (!strcmp(lib->name.str(), name)) {
      return lib;
    }
  }

  // calling code should load mtllib if not loaded
  return 0;
}

internal Material*
find_material(Array<Material_Library *>libs, const char *id) {
  for (u32 i = 0; i < libs.size(); i++) {
    Material_Library *lib = libs[i];
    for (u32 mat_index = 0; mat_index < lib->materials_count; mat_index++) {
      Material *mat = &lib->materials[mat_index];
      if (!strcmp(id, mat->id.str())) {
        return mat;
      }
    }
  }
  
  log_error("find_material(): could not find material\n");
  ASSERT(0);
  return 0;
}

internal void
obj_fill_arrays(Obj_File *obj, File file, Geometry *model) {
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

  u32 materials_index = 0;
  String path = get_path(file.path);

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
      case OBJ_TOKEN_MTLLIB: {
        // loading the material library to the global assets
        Material_Library *mtllib = 0;
        mtllib = find_mtllib(obj->token.lexeme);
        if (!mtllib) {
          mtllib = (Material_Library *)asset_array_next(&assets.mtllibs);
          load_mtl(mtllib, path.str(), obj->token.lexeme);
        }

        model->mtllibs.insert(mtllib);
      } break;
      case OBJ_TOKEN_USEMTL: {
        meshes_index++;
        obj->token.print();
        // @WARNING dependent on the geometry always using the same material library
        model->meshes[meshes_index].material = find_material(model->mtllibs, obj->token.lexeme);
      }
    }
/*
    else if (token.type == OBJ_TOKEN_LINE)
    {
        token = scan_obj(&file, &line_num, true, buffer);
        token = scan_obj(&file, &line_num, true, buffer);
    }
*/
  } while(obj->token.type != OBJ_TOKEN_EOF);

  path.destroy();
}

internal void
model_create_meshes(Geometry *model, Obj_File obj) {
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
      Obj_Face_Vertex *face = &obj.face_vertices[i];
      vertex->position   = obj.vertices[face->position_index - 1];
      vertex->normal     = obj.normals [face->normal_index   - 1];
      vertex->uv         = obj.uvs     [face->uv_index       - 1];
    }
    
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

  model_create_meshes(&geo, obj);

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

    if (mesh->material) {
      if (mesh->material->diffuse_map.memory) {
        vulkan_create_texture(&mesh->material->diffuse_map, TEXTURE_PARAMETERS_DEFAULT);
      }
    }
  }
}

internal void
draw_geometry_no_material(Geometry *geo) {
  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    vulkan_draw_mesh(mesh);
  }
}

internal void
draw_geometry(Geometry *geo) {
  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    Material *mat = mesh->material;

    Material_Shader m_s = mat->shader();

    Local local = {};

    gfx_bind_descriptor_set(GFXID_MATERIAL, &m_s);
    if (mat->diffuse_map.memory) {
      gfx_bind_bitmap(GFXID_TEXTURE, &mat->diffuse_map, 0);
      local.text.x = 2;
    }
    gfx_bind_descriptor_set(GFXID_LOCAL, &local);

    vulkan_draw_mesh(mesh);
  }
}

// @WARNING: Not correct handling of materials
internal void
destroy_geometry(Geometry *geo) {
  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    //vulkan_init_mesh(mesh);

    if (mesh->material) {
      if (mesh->material->diffuse_map.memory) {
        vulkan_destroy_texture(&mesh->material->diffuse_map);
      }
    }
  }
}

internal void
load_geometry(u32 id, const char *filename) {
  String filepath = String(asset_folders[AT_GEOMETRY], filename);
  File file = load_file(filepath.str());
  filepath.destroy();

  Geometry *geo = find_geometry(id);
  *geo = load_obj(file);
}

internal Vector3
geometry_size(Geometry *geo) {
  Vector3 max = {};
  Vector3 min = {};

  for (u32 i = 0; i < geo->meshes_count; i++) {
    Mesh *mesh = &geo->meshes[i];
    Vertex_XNU *vertices = (Vertex_XNU *)mesh->vertices;

    for (u32 vertex_i = 0; vertex_i < mesh->vertices_count; vertex_i++) {
      Vector3 pos = vertices[vertex_i].position;

      for (u32 axis_i = 0; axis_i < 3; axis_i++) {
        float32 value = pos.E[axis_i];
        if (value > max.E[axis_i])
          max.E[axis_i] = value;
        if (value < min.E[axis_i])
          min.E[axis_i] = value;
      }
    }
  }

  Vector3 size = max - min;
  return size;
}