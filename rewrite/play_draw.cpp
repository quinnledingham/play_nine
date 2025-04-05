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


internal Quaternion
get_card_rotation(float32 x_rot, float32 y_rot, bool8 flipped) {
  if (flipped)
    x_rot += PI;

  Quaternion x_quat = get_rotation(x_rot, X_AXIS);
  Quaternion y_quat = get_rotation(y_rot, Y_AXIS);
  Quaternion rotation = y_quat * x_quat;

  return rotation;
}

internal Transform
get_pile_pose(Game *game, Game_Draw *draw, Vector3 offset, float32 yaw) {
  float32 center_of_piles_z = draw->x_hand_position + draw->pile_distance_from_hand;
  float32 rad = draw->player_hand_rads[game->active_player];
  float32 rotation_rads = rad - (0.0f * DEG2RAD);
  
  Transform pose = {};
  pose.yaw = yaw;
  pose.phi = rad;
  pose.z = center_of_piles_z;
  pose.position += offset;
  rotate_coords(&pose.position, rotation_rads);

  return pose;
}

internal void
set_cards_coords(Game *game, Game_Draw *draw) {

  //
  // player cards
  //

  for (u32 p_i = 0; p_i < game->players_count; p_i++) {
    float32 y_rot = draw->player_hand_rads[p_i];

    for (u32 c_i = 0; c_i < HAND_SIZE; c_i++) {
      Card_Entity *e = draw->card_entities.open();
      e->transform.position = get_card_position(c_i, p_i, y_rot);
      e->transform.phi = y_rot;
      e->transform.yaw = PI; // flipped over
      e->index = game->players[p_i].cards[c_i].index;
      game->players[p_i].cards[c_i].entity = e;
    }
  }

  //
  // piles
  //  

  draw->draw_pile_entity    = draw->card_entities.open();
  draw->discard_pile_entity = draw->card_entities.open();

  draw->draw_pile_entity->transform    = get_pile_pose(game, draw, draw->draw_pile_offset,      PI);
  draw->draw_pile_entity->index        = game->draw_pile.top_card();
  draw->discard_pile_entity->transform = get_pile_pose(game, draw, draw->discard_pile_offset, 0.0f);
  draw->discard_pile_entity->index     = game->discard_pile.top_card();
};

internal void
init_game_draw(Game *game, Game_Draw *draw) {
  if (game->players_count == 0) {
    ASSERT(0);
  }

  draw->card_dim = geometry_size(find_geometry(GEOMETRY_CARD)).xz();

  init_relative_hand_coords(draw);
  init_absolute_hand_coords(game, draw);
  draw->x_hand_position = draw->absolute_hand_coords[0].x;

  draw->card_entities.init(100);

  set_cards_coords(game, draw);

  draw->hitbox = get_cube(false, {game_draw.card_dim.x, 0.05f, game_draw.card_dim.y});
}

// draws a individual card
internal void
draw_card(Pose pose, s32 value) {
  if (value == -5)
    value = 13;

  Geometry *geo = find_geometry(GEOMETRY_CARD);
  Geometry *side = find_geometry(GEOMETRY_CARD_SIDE);

  Material_Shader m_s = geo->meshes[0].material->shader();
  gfx_ubo(GFXID_MATERIAL, &m_s, 0);

  gfx_bind_bitmap(GFXID_TEXTURE, BITMAP_BACK, 0);

  Local local = {};
  local.text.x = 2.0f;
  gfx_bind_descriptor_set(GFXID_LOCAL, &local);

  float32 thickness = 0.05f;
  float32 half = thickness / 2.0f;

  // draw 
  {
    Pose bottom = pose;

    bottom.roll -= PI;

    if (bottom.roll == 0.0f) {
      bottom.position += Vector3{0, half, 0};
    } else {
      bottom.position -= Vector3{0, half, 0};
    }

    Object object = {};
    object.model = m4x4(bottom, {1.0f, 1.0f, 1.0f});
    vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
    draw_geometry_no_material(geo);
  }

  Texture_Atlas *atlas = find_atlas(ATLAS_CARDS);
  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);
  local.text.x = 3.0f;
  local.region = atlas_region(atlas, value);
  gfx_ubo(GFXID_LOCAL, &local, 0);

  {
    Pose top = pose;

    if (top.roll == 0.0f) {
      top.position -= Vector3{0, half, 0};
    } else {
      top.position += Vector3{0, half, 0};
    }

    Object object = {};
    object.model = m4x4(top, {1.0f, 1.0f, 1.0f});
    vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
    draw_geometry_no_material(geo);
  }

  // Sides
  {
    local.color = game_draw.card_side_color;
    local.text.x = 0.0f;
    gfx_ubo(GFXID_LOCAL, &local, 0);
    
    Object object = {};
    object.model = m4x4(pose, {1.0f, thickness, 1.0f});
    vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
    draw_geometry_no_material(side);
  }
}

internal void
draw_cards(Game *game, Game_Draw *draw) {
  float32 thickness = 0.05f;
  float32 half = thickness / 2.0f;

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
  
  Quaternion flip = get_rotation(PI, X_AXIS);

  // back side
  for (u32 i = 0; i < draw->card_entities.size(); i++) {
    Card_Entity *e = draw->card_entities.get(i);
    if (e) {
      Transform t = e->transform;
      
      t.roll -= PI;

      Vector3 displacement = rotation(t.orientation) * Vector3{0, half, 0};
      t.position += displacement;

      if (e->hovered) {
        t.position.y += 2.0f;
      }

      Object object = {};
      object.model = m4x4(t);
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

      vkCmdDrawIndexed(VK_CMD, face_mesh->indices_count, 1, 0, 0, 0);
    }
  }

  // front side
  Texture_Atlas *atlas = find_atlas(ATLAS_CARDS);
  vulkan_bind_descriptor_set(atlas->gpu[vk_ctx.current_frame].set);
  local.text.x = 3.0f;

  for (u32 i = 0; i < draw->card_entities.size(); i++) {
    Card_Entity *e = draw->card_entities.get(i);
    if (e) {
      Transform t = e->transform;

      Vector3 displacement = rotation(t.orientation) * Vector3{0, -half, 0};
      t.position -= displacement;

      s32 value = deck[e->index];
      if (value == -5)
        value = 13;
      local.region = atlas_region(atlas, value);
      gfx_ubo(GFXID_LOCAL, &local, 0);

      Object object = {};
      object.model = m4x4(t);
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

      vkCmdDrawIndexed(VK_CMD, face_mesh->indices_count, 1, 0, 0, 0);
    }
  }

  vulkan_bind_mesh(side_mesh);
  local.color = game_draw.card_side_color;
  local.text.x = 0.0f;
  gfx_ubo(GFXID_LOCAL, &local, 0);

  for (u32 i = 0; i < draw->card_entities.size(); i++) {
    Card_Entity *e = draw->card_entities.get(i);
    if (e) {
      Transform t = e->transform;
      t.scale = {1.0f, thickness, 1.0f};

      Object object = {};
      object.model = m4x4(t);
      vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));

      vkCmdDrawIndexed(VK_CMD, side_mesh->indices_count, 1, 0, 0, 0);
    }
  }
}

internal void
draw_hitbox(Card_Entity *e) {
  Descriptor_Set local_desc_set = gfx_descriptor_set(GFXID_LOCAL);

  Descriptor color_desc = gfx_descriptor(&local_desc_set, 0);
  Local local = {};
  local.color = {255, 0, 0, 1};
  vulkan_update_ubo(color_desc, (void *)&local);

  vulkan_bind_descriptor_set(local_desc_set);

  Object object = {};
  object.model = m4x4(e->transform);
  object.index = 0;
  vulkan_push_constants(SHADER_STAGE_VERTEX, (void *)&object, sizeof(Object));
  vulkan_draw_mesh(&game_draw.hitbox);
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

  //draw_cube({0, 0, 0}, 0, {2, 2, 2}, {255, 0, 0, 1});

#ifdef DEBUG
  
  if (debug.wireframe)
    draw_ray(&mouse_ray);

#endif // DEBUG

  gfx_scissor_pop();
}

internal Pose
get_player_camera(float32 degrees_between, u32 active_i) {

  float32 target_degrees = (degrees_between * (float32)active_i);

  float32 rad = target_degrees * DEG2RAD;
  float32 cam_dis = 9.0f + game_draw.radius;
  float32 x = cam_dis * cosf(rad);
  float32 y = cam_dis * sinf(rad);

  Pose pose = {};
  pose.x = x;
  pose.y = 12.0f;
  pose.z = y;
  pose.pitch = -44.0f * DEG2RAD;
  pose.yaw = (target_degrees + 180.0f) * DEG2RAD;

  return pose;

}