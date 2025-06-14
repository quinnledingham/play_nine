internal Mesh
get_rect_mesh_3D() {
    Mesh mesh = {};
    mesh.vertices_count = 4;

    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
    Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;

    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
    /*
    vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, -1}, {0, 0} };
    vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, -1}, {0, 1} };
    vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, -1}, {1, 0} };
    vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, -1}, {1, 1} };
    */
    mesh.indices_count = 6;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    // vertex locations
    u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
    /* 
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_left;
    mesh.indices[2] = bottom_right;
    mesh.indices[3] = top_left;
    mesh.indices[4] = bottom_right;
    mesh.indices[5] = top_right;
    */

    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_right;
    mesh.indices[2] = bottom_left;
    mesh.indices[3] = top_left;
    mesh.indices[4] = top_right;
    mesh.indices[5] = bottom_right;

    mesh.vertex_info = Vertex_XNU::info();
    vulkan_init_mesh(&mesh);

    return mesh;
}

internal Mesh
get_rect_mesh_2D() {
  Mesh mesh = {};
  mesh.vertices_count = 4;
  
  mesh.vertices = ARRAY_MALLOC(Vertex_XU, mesh.vertices_count);
  Vertex_XU *vertices = (Vertex_XU *)mesh.vertices;

  //vertices[0] = Vertex_XU{ {-0.5, -0.5 }, {0, 0} };
  //vertices[1] = Vertex_XU{ {-0.5,  0.5 }, {0, 1} };
  //vertices[2] = Vertex_XU{ { 0.5, -0.5 }, {1, 0} };
  //vertices[3] = Vertex_XU{ { 0.5,  0.5 }, {1, 1} };

  vertices[0] = Vertex_XU{ { 0, 0 }, {0, 0} };
  vertices[1] = Vertex_XU{ { 0, 1 }, {0, 1} };
  vertices[2] = Vertex_XU{ { 1, 0 }, {1, 0} };
  vertices[3] = Vertex_XU{ { 1, 1 }, {1, 1} };

  mesh.indices_count = 6;
  mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

  // vertex locations
  u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
 
  mesh.indices[0] = top_left;
  mesh.indices[1] = bottom_right;
  mesh.indices[2] = bottom_left;
  mesh.indices[3] = top_left;
  mesh.indices[4] = top_right;
  mesh.indices[5] = bottom_right;

  mesh.vertex_info = Vertex_XU::info();
  vulkan_init_mesh(&mesh);

  return mesh;
}

internal void
init_rect_indices(u32 *indices, u32 top_left,    u32 top_right,
                                u32 bottom_left, u32 bottom_right) {
    indices[0] = top_left;
    indices[1] = bottom_right;
    indices[2] = bottom_left;
    indices[3] = top_left;
    indices[4] = top_right;
    indices[5] = bottom_right;
}

internal Mesh
get_cube(bool8 out, Vector3 coords, Vector3 scale) {
  Mesh mesh = {};
  
  mesh.vertices_count = 8;
  mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);
  Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;
  
  Vector3 s = scale / 2.0f;

  // back
  vertices[0] = { {-s.x, -s.y, -s.z }, {  1,  1, 1 }, {0, 0} }; // bottom left
  vertices[1] = { {-s.x,  s.y, -s.z }, {  1, -1, 1 }, {0, 1} }; // top left
  vertices[2] = { { s.x, -s.y, -s.z }, { -1,  1, 1 }, {1, 0} }; // bottom right
  vertices[3] = { { s.x,  s.y, -s.z }, { -1, -1, 1 }, {1, 1} }; // top right
  
  // forward
  vertices[4] = { {-s.x, -s.y, s.z }, {  1,  1, -1 }, {0, 0} }; // bottom left
  vertices[5] = { {-s.x,  s.y, s.z }, {  1, -1, -1 }, {0, 1} }; // top left
  vertices[6] = { { s.x, -s.y, s.z }, { -1,  1, -1 }, {1, 0} }; // bottom right
  vertices[7] = { { s.x,  s.y, s.z }, { -1, -1, -1 }, {1, 1} }; // top right

  mesh.indices_count = 6 * 6; // 6 indices per side (rects), 6 sides
  mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
  
  if (out) {
    init_rect_indices(mesh.indices + 0,  3, 1, 2, 0); // back
    init_rect_indices(mesh.indices + 6,  5, 7, 4, 6); // front
    init_rect_indices(mesh.indices + 12, 1, 3, 5, 7); // top
    init_rect_indices(mesh.indices + 18, 4, 6, 0, 2); // bottom
    init_rect_indices(mesh.indices + 24, 1, 5, 0, 4); // left
    init_rect_indices(mesh.indices + 30, 7, 3, 6, 2); // right
  } else {
    init_rect_indices(mesh.indices + 0,  1, 3, 0, 2); // back
    init_rect_indices(mesh.indices + 6,  7, 5, 6, 4); // front
    init_rect_indices(mesh.indices + 12, 5, 7, 1, 3); // top
    init_rect_indices(mesh.indices + 18, 0, 2, 4, 6); // bottom
    init_rect_indices(mesh.indices + 24, 5, 1, 4, 0); // left
    init_rect_indices(mesh.indices + 30, 3, 7, 2, 6); // right
  }

  mesh.vertex_info = Vertex_XNU::info();
  vulkan_init_mesh(&mesh);
  
  return mesh;
}

internal void 
draw_cube(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);

  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, get_rotation(0, {0, 1, 0}), dim);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.cube);
}

internal Mesh 
get_sphere(float32 radius, u32 u_subdivision, u32 v_subdivision) {
    Mesh mesh = {};

    float32 u_degrees = PI;
    float32 v_degrees = 2.0f * PI;

    float32 u_step = u_degrees / (float32)u_subdivision;
    float32 v_step = v_degrees / (float32)v_subdivision;

    mesh.vertices_count = (u_subdivision + 1) * (v_subdivision + 1);
    mesh.vertices = ARRAY_MALLOC(Vertex_XNU, mesh.vertices_count);

    u32 vertices_index = 0;
    for (u32 u = 0; u <= u_subdivision; ++u) {
        for (u32 v = 0; v <= v_subdivision; ++v) {
            float32 u_f = (float32)u * u_step;
            float32 v_f = (float32)v * v_step;

            float32 inverse_radius = 1.0f / radius;
            Vector3 position = { 
                radius * sinf(u_f) * cosf(v_f), 
                radius * sinf(u_f) * sinf(v_f), 
                radius * cosf(u_f) 
            };
            Vector3 normal = position * inverse_radius;
            Vector2 texture_coords = { 
                1 - (v_f / v_degrees), 
                u_f / u_degrees
            };

            Vertex_XNU *vertices = (Vertex_XNU *)mesh.vertices;
            vertices[vertices_index++] = { position, normal, texture_coords };
        }
    }

    mesh.indices_count = (u_subdivision * v_subdivision * 6) - (u_subdivision * 6);
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    u32 indices_index = 0;
    for (u32 u = 0; u < u_subdivision; u++) {
        u32 p1 = u * (v_subdivision + 1);
        u32 p2 = p1 + v_subdivision + 1;

        for (u32 v = 0; v < v_subdivision; v++, p1++, p2++) {
            if (u != 0) {
                mesh.indices[indices_index++] = p1;
                mesh.indices[indices_index++] = p2;
                mesh.indices[indices_index++] = p1 + 1;
            } 
            if (u != (u_subdivision - 1)) {
                mesh.indices[indices_index++] = p1 + 1;
                mesh.indices[indices_index++] = p2;
                mesh.indices[indices_index++] = p2 + 1;
            }
        }
    }

    mesh.vertex_info = Vertex_XNU::info();
    vulkan_init_mesh(&mesh);

    return mesh;
}

internal void 
draw_sphere(Vector3 coords, float32 rotation, Vector3 dim, Vector4 color) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);

  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, get_rotation(0, {0, 1, 0}), dim);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.sphere);
}

// From Clay
internal Mesh
draw_rounded_rect(Vector2 coords, Vector2 size, Vector4 color, const float corner_radius) {
  Rect rect = {};
  rect.coords = coords;
  rect.dim = size;

  const float min_radius = SDL_min(rect.w, rect.h) / 2.0f;
  const float clamped_radius = SDL_min(corner_radius, min_radius);
  const int num_circle_segments = SDL_max(16, (int) (clamped_radius * 0.5f));

  Mesh mesh = {};
  mesh.vertices_count = 4 + (4 * (num_circle_segments * 2)) + 2*4;
  mesh.indices_count = 6 + (4 * (num_circle_segments * 3)) + 6*4;

  mesh.vertices = ARRAY_MALLOC(Vertex_XU, mesh.vertices_count);
  mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

  u32 vertices_index = 0;
  u32 indices_index = 0;

  Vertex_XU *vertices = (Vertex_XU *)mesh.vertices;
  u32 *indices = (u32 *)mesh.indices;

  vertices[vertices_index++] = Vertex_XU{ { rect.x + clamped_radius, rect.y + clamped_radius},                   { 0, 0 } };
  vertices[vertices_index++] = Vertex_XU{ { rect.x + rect.w - clamped_radius, rect.y + clamped_radius},          { 1, 0 } };
  vertices[vertices_index++] = Vertex_XU{ { rect.x + rect.w - clamped_radius, rect.y + rect.h - clamped_radius}, { 1, 1 } };
  vertices[vertices_index++] = Vertex_XU{ { rect.x + clamped_radius, rect.y + rect.h - clamped_radius},          { 0, 1 } };

  indices[indices_index++] = 0;
  indices[indices_index++] = 1;
  indices[indices_index++] = 3;
  indices[indices_index++] = 1;
  indices[indices_index++] = 2;
  indices[indices_index++] = 3;

  //define rounded corners as triangle fans
  const float step = (SDL_PI_F/2) / num_circle_segments;
  for (int i = 0; i < num_circle_segments; i++) {
    const float angle1 = (float)i * step;
    const float angle2 = ((float)i + 1.0f) * step;

    for (int j = 0; j < 4; j++) {  // Iterate over four corners
      float cx, cy, signX, signY;

      switch (j) {
        case 0: cx = rect.x + clamped_radius;          cy = rect.y + clamped_radius;          signX = -1; signY = -1; break; // Top-left
        case 1: cx = rect.x + rect.w - clamped_radius; cy = rect.y + clamped_radius;          signX =  1; signY = -1; break; // Top-right
        case 2: cx = rect.x + rect.w - clamped_radius; cy = rect.y + rect.h - clamped_radius; signX =  1; signY =  1; break; // Bottom-right
        case 3: cx = rect.x + clamped_radius;          cy = rect.y + rect.h - clamped_radius; signX = -1; signY =  1; break; // Bottom-left
        default:
          log_error("create_rounded_rect(): PROBLEM?\n");
          return mesh;
      }

      vertices[vertices_index++] = Vertex_XU{ {cx + SDL_cosf(angle1) * clamped_radius * signX, cy + SDL_sinf(angle1) * clamped_radius * signY}, {0, 0} };
      vertices[vertices_index++] = Vertex_XU{ {cx + SDL_cosf(angle2) * clamped_radius * signX, cy + SDL_sinf(angle2) * clamped_radius * signY}, {0, 0} };

      if (j == 0 || j == 2) {
        indices[indices_index++] = j;  // Connect to corresponding central rectangle vertex
        indices[indices_index++] = vertices_index - 2;
        indices[indices_index++] = vertices_index - 1;
      } else {
        indices[indices_index++] = j;  // Connect to corresponding central rectangle vertex]
        indices[indices_index++] = vertices_index - 1;
        indices[indices_index++] = vertices_index - 2;
      }
    }
  }

  //Define edge rectangles
  // Top edge
  vertices[vertices_index++] = Vertex_XU{ {rect.x + clamped_radius, rect.y}, {0, 0} }; //TL
  vertices[vertices_index++] = Vertex_XU{ {rect.x + rect.w - clamped_radius, rect.y}, {1, 0} }; //TR

  indices[indices_index++] = 0;
  indices[indices_index++] = vertices_index - 2; //TL
  indices[indices_index++] = vertices_index - 1; //TR
  indices[indices_index++] = 1;
  indices[indices_index++] = 0;
  indices[indices_index++] = vertices_index - 1; //TR
  // Right edge
  vertices[vertices_index++] = Vertex_XU{ {rect.x + rect.w, rect.y + clamped_radius}, {1, 0} }; //RT
  vertices[vertices_index++] = Vertex_XU{ {rect.x + rect.w, rect.y + rect.h - clamped_radius}, {1, 1} }; //RB

  indices[indices_index++] = 1;
  indices[indices_index++] = vertices_index - 2; //RT
  indices[indices_index++] = vertices_index - 1; //RB
  indices[indices_index++] = 2;
  indices[indices_index++] = 1;
  indices[indices_index++] = vertices_index - 1; //RB
  // Bottom edge
  vertices[vertices_index++] = Vertex_XU{ {rect.x + rect.w - clamped_radius, rect.y + rect.h}, {1, 1} }; //BR
  vertices[vertices_index++] = Vertex_XU{ {rect.x + clamped_radius, rect.y + rect.h}, {0, 1} }; //BL

  indices[indices_index++] = 2;
  indices[indices_index++] = vertices_index - 2; //BR
  indices[indices_index++] = vertices_index - 1; //BL
  indices[indices_index++] = 3;
  indices[indices_index++] = 2;
  indices[indices_index++] = vertices_index - 1; //BL
  // Left edge
  vertices[vertices_index++] = Vertex_XU{ {rect.x, rect.y + rect.h - clamped_radius}, {0, 1} }; //LB
  vertices[vertices_index++] = Vertex_XU{ {rect.x, rect.y + clamped_radius}, {0, 0} }; //LT

  indices[indices_index++] = 3;
  indices[indices_index++] = vertices_index - 2; //LB
  indices[indices_index++] = vertices_index - 1; //LT
  indices[indices_index++] = 0;
  indices[indices_index++] = 3;
  indices[indices_index++] = vertices_index - 1; //LT

  mesh.vertex_info = Vertex_XU::info();
  //vulkan_init_mesh(&mesh);

  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = identity_m4x4();
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

  vulkan_draw_immediate_mesh(&mesh);

  free(mesh.vertices);
  free(mesh.indices);

  return mesh;
}

internal void
draw_rect(Vector2 coords, Vector2 size, Vector4 color) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square);
}

internal void
draw_rect(Vector2 coords, Vector2 size, Vulkan_Texture *texture) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.text.x = 2;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Descriptor_Set texture_desc_set = gfx_descriptor_set(GFXID_TEXTURE);
  Descriptor texture_desc = gfx_descriptor(&texture_desc_set, 0);
  vulkan_set_texture(&texture_desc, texture);
  vulkan_bind_descriptor_set(texture_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square);
}

internal void
draw_rect(Vector2 coords, Vector2 size, Bitmap *bitmap) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.text.x = 2;
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  Descriptor_Set texture_desc_set = gfx_descriptor_set(GFXID_TEXTURE);
  Descriptor texture_desc = gfx_descriptor(&texture_desc_set, 0);
  vulkan_set_bitmap(&texture_desc, bitmap);
  vulkan_bind_descriptor_set(texture_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square);
}

internal void
draw_rect(Vector2 coords, Vector2 size, Texture_Atlas *atlas, u32 index) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.text.x = 3;
  local.region = atlas_region(atlas, index);
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);

  Object object = {};
  object.model = create_transform_m4x4(coords, size);
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square);
}

internal void
draw_rect_3D(Vector3 coords, Vector3 size, Vector4 color) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = color;
  vulkan_update_ubo(color_desc, (void *)&local);

  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = create_transform_m4x4(coords, get_rotation(0, {0, 1, 0}), size);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&draw_ctx.square_3D);
}

internal void
draw_text_baseline(u32 id, const char *text, Vector2 coords, float32 pixel_height, Vector4 color) {
  Font *font = find_font(id);
  float32 scale = get_scale_for_pixel_height(font, pixel_height);
  if (scale == 0.0f)
    return;

  float32 string_x_coord = 0.0f;
  float32 current_point  = coords.x;
  float32 baseline       = coords.y;

  Font_Char *font_char = 0;
  Font_Char *font_char_next = 0;
  u32 text_index = 0;

  Object object = {};
  object.model = identity_m4x4();
  vulkan_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

  Texture_Atlas *atlas = &font->cache->atlas;
  
  Local local = {};
  local.text.x = 1;
  local.color = color;

  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);
  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  vulkan_update_ubo(color_desc, (void *)&local);
  vulkan_bind_descriptor_set(local_desc_set);

  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);

  while(text[text_index] != 0) {
    Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, text[text_index], scale);

    font_char = bitmap->font_char;
    font_char_next = load_font_char(font, text[text_index + 1]);

    // Draw
    if (bitmap->bitmap.width != 0) {
      Vector2 coords = { 
        current_point + (font_char->lsb * scale), 
        baseline + (float32)bitmap->bb_0.y 
      };
      Vector2 dim = { 
        (float32)bitmap->bitmap.width, 
        (float32)bitmap->bitmap.height 
      };
      
      u32 index = bitmap->index;
      Texture_Coords tex_coord = atlas->texture_coords[index];
  
      vulkan_immediate_vertex_xu(Vertex_XU{ coords, tex_coord.p1 });
      vulkan_immediate_vertex_xu(Vertex_XU{ coords + dim, tex_coord.p2 });
      vulkan_immediate_vertex_xu(Vertex_XU{ { coords.x, coords.y + dim.y }, {tex_coord.p1.x, tex_coord.p2.y} });

      vulkan_immediate_vertex_xu(Vertex_XU{ coords, tex_coord.p1 });
      vulkan_immediate_vertex_xu(Vertex_XU{ { coords.x + dim.x, coords.y }, {tex_coord.p2.x, tex_coord.p1.y} });
      vulkan_immediate_vertex_xu(Vertex_XU{ coords + dim, tex_coord.p2 });

      vulkan_draw_immediate(6);
    }

    // End of Draw
    s32 kern = get_glyph_kern_advance(font->info, font_char->glyph_index, font_char_next->glyph_index);
    current_point += scale * (kern + font_char->ax);

    text_index++;
  }
}

/*
(0, 0)
 -> ###############################
    #   #                         #
    # r #            r            #
    # b #                         #  
    #   #                         #
    ###############################
    # r #            r            #
    ############################### 
*/
struct String_Draw_Info {
    Vector2 dim;      // dim of the text
    Vector2 baseline;

    Vector2 font_dim; // biggest char in font
    Vector2 font_baseline; // baseline for biggest char
};

// if length != -1 than the dim only includes chars up to the length position
internal String_Draw_Info
get_string_draw_info(Font *font, const char *string, s32 length, float32 pixel_height) {
  String_Draw_Info info = {};    

  if (string == 0) {
    return info;
  } else if (font == 0) {
    log_error("get_string_draw_info() no font\n");
    return info;
  }

  stbtt_fontinfo *font_info = (stbtt_fontinfo*)font->info;
  float32 scale = stbtt_ScaleForPixelHeight(font_info, pixel_height);
  u32 index = 0;

  s32 top = 0;
  s32 bottom = 0;

  while (string[index] != 0) {
    if (length != -1) {
      // if a length is set then only include in the dim the chars up to that point
      if (index == length) 
        break;
    }

    Font_Char_Bitmap *font_char_bitmap = load_font_char_bitmap(font, string[index], scale);
    Font_Char *font_char = font_char_bitmap->font_char;

    // where should drawing start on the x
    if (index == 0) {
      float32 lsb_scaled = roundf(font_char->lsb * scale);
      s32 char_coords_x = font_char_bitmap->bb_0.x;
      info.baseline.x = -float32(lsb_scaled);
      info.dim.x = -float32(char_coords_x);
    }

    if (top > font_char_bitmap->bb_0.y)
      top = font_char_bitmap->bb_0.y;

    if (bottom < font_char_bitmap->bb_1.y)
      bottom = font_char_bitmap->bb_1.y;
    
    if ((float32)-font_char_bitmap->bb_0.y > info.baseline.y)
      info.baseline.y = (float32)-font_char_bitmap->bb_0.y;

    int kern = stbtt_GetCodepointKernAdvance(font_info, string[index], string[index + 1]);
    if (string[index + 1] || string[index] == 32) { // 32 == ' ' bitmap width is 0 so need advance
      info.dim.x += scale * float32(kern + font_char->ax);
    } else {
      // last character so just add the width of the character
      info.dim.x += float32(font_char_bitmap->bb_1.x);
    }
    index++;
  }

  info.dim.y = float32(bottom - top);
      
  s32 x0, y0, x1, y1;
  stbtt_GetFontBoundingBox(font_info, &x0, &y0, &x1, &y1);
  info.font_dim.x = float32(x1 - x0) * scale;
  info.font_dim.y = float32(y1 - y0) * scale;
  info.font_baseline.x = float32(x0) * scale;
  info.font_baseline.y = float32(y1) * scale;
  
  return info;
}

internal String_Draw_Info
get_string_draw_info(u32 id, const char *string, s32 length, float32 pixel_height) {
  Font *font = find_font(id);
  return get_string_draw_info(font, string, length, pixel_height);
}

internal void
draw_text(const char *text, Vector2 coords, float32 pixel_height, Vector4 color) {
  String_Draw_Info string_info = get_string_draw_info(draw_ctx.font_id, text, -1, pixel_height);

#ifdef DEBUG
  
  if (debug.draw_text_info) {
    draw_rect(coords, string_info.dim, { 100, 0, 0, 0.5 });
    draw_rect(coords + string_info.baseline, { 5, 5 }, { 0, 100, 0, 1 });
  }

#endif // DEBUG

  coords += string_info.baseline;
  draw_text_baseline(draw_ctx.font_id, text, coords, pixel_height, color);
}

internal void
init_draw() {
  draw_ctx.square = get_rect_mesh_2D();
  draw_ctx.square_3D = get_rect_mesh_3D();
  draw_ctx.sphere = get_sphere(0.025f, 10, 10);
  draw_ctx.cube = get_cube(false, {0, 0, 0}, {1, 1, 1});
  Rect r = {};
  r.coords = {0, 0};
  r.dim = {1, 1};
  //draw_ctx.rounded_rect = create_rounded_rect(r);
}

inline void
set_draw_font(u32 id) {
  draw_ctx.font_id = id;

  Font *font = find_font(id);
  Texture_Atlas *atlas = &font->cache->atlas;
}

internal void
start_draw_2D() {
  gfx_default_viewport();
  gfx_scissor_push({0, 0}, {(float32)gfx.window.dim.width, (float32)gfx.window.dim.height});
  vulkan_depth_test(false);

  gfx_bind_pipeline(PIPELINE_2D);

  Descriptor_Set scene_desc_set = gfx_descriptor_set(GFXID_SCENE);
  Descriptor scene_desc = gfx_descriptor(&scene_desc_set, 0);
  vulkan_update_ubo(scene_desc, &ortho_scene);
  vulkan_bind_descriptor_set(scene_desc_set);

  // Default texture
  Bitmap *bitmap = find_bitmap(BITMAP_LANA);
  Descriptor_Set texture_desc_set = gfx_descriptor_set(GFXID_TEXTURE);
  Descriptor texture_desc = gfx_descriptor(&texture_desc_set, 0);
  vulkan_set_bitmap(&texture_desc, bitmap);
  vulkan_bind_descriptor_set(texture_desc_set);

}

internal void
end_draw_2D() {
  gfx_scissor_pop();
}
