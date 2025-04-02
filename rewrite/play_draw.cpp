/*
  -2 -1 0 1, 0
  -2 -1 0 1, 1
*/
internal void
init_relative_hand_coords(Game_Draw *draw) {
  ASSERT(HAND_SIZE == 8);

  //float32 card_width = 2.0f;
  //float32 card_height = 3.2f;
  float32 padding = 0.2f;
  draw->hand_width = draw->card_dim.width * 4.0f + padding * 3.0f;

  u32 card_index = 0;
  for (s32 y = 1; y >= 0; y--) {
      for (s32 x = -2; x <= 1; x++) {
          draw->relative_hand_coords[card_index++] = { 
              float32(x) * draw->card_dim.width  + (float32(x) * padding) + (draw->card_dim.width  / 2.0f) + (padding / 2.0f), 
              float32(y) * draw->card_dim.height + (float32(y) * padding) - (draw->card_dim.height / 2.0f) - (padding / 2.0f)
          };
      }
  }
}

// -deg: negative because I want to go counter clockwise but still
// have degrees between players be positive
internal Vector3
get_absolute_hand_position(float32 hyp, float32 deg, u32 i) {
    float32 rad = -deg * DEG2RAD;
    Vector3 position = { 0, 0, 0 };
    position.x = hyp * cosf(i * rad);
    position.z = hyp * sinf(i * rad);
    return position;
}

internal float32
get_draw_radius(u32 num_of_players, float32 hand_width, float32 card_height) {
    if (num_of_players == 2) {
        return 2.0f * card_height;
    } 

    float32 angle = (360.0f / float32(num_of_players)) / 2.0f;
    float32 radius = (hand_width / 2.0f) / tanf(angle * DEG2RAD);
    radius += card_height + 0.1f;

    if (num_of_players == 3) {
        radius += 1.0f;
    }

    return radius;
}

internal void
init_absolute_hand_coords(Game *game, Game_Draw *draw) {
  u32 draw_num_of_players = game->players_count;
  if (draw_num_of_players == 1)
      draw_num_of_players = 2;
  draw->degrees_between_players = 360.0f / float32(draw_num_of_players);
  draw->radius = get_draw_radius(draw_num_of_players, draw->hand_width, draw->card_dim.height);

  for (u32 player_i = 0; player_i < game->players_count; player_i++) {
    float32 degrees = (draw->degrees_between_players * player_i) + 270.0f;
    draw->player_hand_rads[player_i] = degrees * DEG2RAD; 

    draw->absolute_hand_coords[player_i] = get_absolute_hand_position(draw->radius, draw->degrees_between_players, player_i);
  }
}

internal void
init_game_draw(Game *game, Game_Draw *draw) {
  if (game->players_count == 0) {
    ASSERT(0);
  }

  draw->card_dim = geometry_size(find_geometry(GEOMETRY_CARD)).xz();

  init_relative_hand_coords(draw);
  init_absolute_hand_coords(game, draw);
}

internal void
rotate_coords(Vector3 *coords, float32 rad) {
    *coords = { 
         cosf(rad) * coords->x + sinf(rad) * coords->z, 
         coords->y, 
        -sinf(rad) * coords->x + cosf(rad) * coords->z 
    };
}

internal Vector3
get_card_position(u32 card_index, u32 active_p, float32 y_axis_rad) {
    Vector3 card_position = {};
    card_position.x = game_draw.relative_hand_coords[card_index].x;
    card_position.z = game_draw.relative_hand_coords[card_index].y;
    rotate_coords(&card_position, y_axis_rad);
    card_position += game_draw.absolute_hand_coords[active_p];
    return card_position;
}

internal void
load_player_hand_models(Game_Draw *draw, u32 player_index) {
  
}

internal Quaternion
get_card_rotation(float32 x_rot, float32 y_rot, bool8 flipped) {
  if (flipped)
    x_rot += PI;

  Quaternion x_quat = get_rotation(x_rot, X_AXIS);
  Quaternion y_quat = get_rotation(y_rot, Y_AXIS);
  Quaternion rotation = y_quat * x_quat;

  return rotation;
}

// draws a individual card
internal void
draw_card(Vector3 coords, float32 x_rot, float32 y_rot) {
  Geometry *geo = find_geometry(GEOMETRY_CARD);
  Geometry *side = find_geometry(GEOMETRY_CARD_SIDE);

  float32 thickness = 0.05f;

  Quaternion y_quat = get_rotation(y_rot, Y_AXIS);
  Quaternion x_quat = get_rotation(x_rot, X_AXIS);
  Quaternion x_quat_flip = get_rotation(x_rot + PI, X_AXIS);

  Quaternion rotation = y_quat * x_quat;
  Quaternion rotation_flip = y_quat * x_quat_flip;

  Object object = {};
  object.model = create_transform_m4x4(coords + Vector3{0, thickness, 0}, rotation, {1.0f, 1.0f, 1.0f});
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  draw_geometry(geo);

  object = {};
  object.model = create_transform_m4x4(coords, rotation_flip, {1.0f, 1.0f, 1.0f});
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  draw_geometry(geo);

  Local local = {};
  local.color = {0, 255, 0, 1};
  gfx_ubo(GFXID_LOCAL, &local, 0);

  object.model = create_transform_m4x4(coords, rotation, {1.0f, thickness, 1.0f});
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  draw_geometry_no_material(side);
}

internal void
draw_cards(Game *game, Game_Draw *draw) {
  float32 thickness = 0.05f;

  Geometry *geo = find_geometry(GEOMETRY_CARD);
  Geometry *side = find_geometry(GEOMETRY_CARD_SIDE);

  Mesh *face_mesh = &geo->meshes[0];
  Mesh *side_mesh = &side->meshes[0];

  Material_Shader m_s = face_mesh->material->shader();
  gfx_ubo(GFXID_MATERIAL, &m_s, 0);

  gfx_bind_bitmap(GFXID_TEXTURE, BITMAP_BACK, 0);

  Local local = {};
  local.text.x = 2.0f;
  gfx_bind_descriptor_set(GFXID_LOCAL, &local);

  vulkan_bind_mesh(face_mesh);
  
  // backside
  for (u32 p_i = 0; p_i < game->players_count; p_i++) {
    float32 x_rot = 0.0f;
    float32 y_rot = draw->player_hand_rads[p_i];
    Quaternion rotation = get_card_rotation(0.0, y_rot, true);

    for (u32 c_i = 0; c_i < HAND_SIZE; c_i++) {
      Vector3 coords = get_card_position(c_i, p_i, y_rot);

      Object object = {};
      object.model = create_transform_m4x4(coords, rotation, {1.0f, 1.0f, 1.0f});
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

      vkCmdDrawIndexed(VK_CMD, face_mesh->indices_count, 1, 0, 0, 0);
    }
  }

  // front side
  Texture_Atlas *atlas = find_atlas(ATLAS_CARDS);
  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);
  local.text.x = 3.0f;

  for (u32 p_i = 0; p_i < game->players_count; p_i++) {
    float32 x_rot = 0.0f;
    float32 y_rot = draw->player_hand_rads[p_i];
    Quaternion rotation = get_card_rotation(0.0, y_rot, false);

    for (u32 c_i = 0; c_i < HAND_SIZE; c_i++) {
      
      Vector3 coords = get_card_position(c_i, p_i, y_rot);

      Object object = {};
      object.model = create_transform_m4x4(coords + Vector3{0, thickness, 0}, rotation, {1.0f, 1.0f, 1.0f});
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
      
      s32 value = deck[game->players[p_i].cards[c_i].index];
      if (value == -5)
        value = 13;
      local.region = atlas_region(atlas, value);
      gfx_ubo(GFXID_LOCAL, &local, 0);

      vkCmdDrawIndexed(VK_CMD, face_mesh->indices_count, 1, 0, 0, 0);
    }
  }

  // sides
  vulkan_bind_mesh(side_mesh);
  local.color = {90, 90, 90, 1};
  local.text.x = 0.0f;
  gfx_ubo(GFXID_LOCAL, &local, 0);

  for (u32 p_i = 0; p_i < game->players_count; p_i++) {
    float32 x_rot = 0.0f;
    float32 y_rot = draw->player_hand_rads[p_i];
    Quaternion rotation = get_card_rotation(0.0, y_rot, false);

    for (u32 c_i = 0; c_i < HAND_SIZE; c_i++) {
      Vector3 coords = get_card_position(c_i, p_i, y_rot);

      Object object = {};
      object.model = create_transform_m4x4(coords, rotation, {1.0f, thickness, 1.0f});
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

      vkCmdDrawIndexed(VK_CMD, side_mesh->indices_count, 1, 0, 0, 0);
    }
  }
}

internal void
draw_game(Game *game) {
  gfx_default_viewport();
  gfx_scissor_push({0, 0}, {(float32)gfx.window.dim.width, (float32)gfx.window.dim.height});
  vulkan_depth_test(true);

  scene.view = get_view(camera);
  gfx_ubo(GFXID_SCENE, &scene.view, 0);

  gfx_bind_pipeline(PIPELINE_3D);

  draw_cards(game, &game_draw);

  gfx_scissor_pop();
}
