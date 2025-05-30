// returns so that the animation can be added on to
internal Animation*
flip_card_animation(Player_Card *card) {
  float32 time_duration = 0.3f;

  Transform *transform = &card->entity->transform;
  Animation *a = find_animation(animations, transform);

  Transform middle = *transform;
  middle.y += 1.0f;
  middle.yaw += PI/2;

  add_keyframe(a, *transform, middle, time_duration/2.0f);

  Transform final = *transform;
  final.yaw += PI;
  final.y = 0.0f;

  add_keyframe(a, *transform, final, time_duration/2.0f);

  return a;
}

bool8 handle_camera_wait(void *args) {
  u32 active_animations = count_active_animations(animations);
  if (active_animations == 1) // only this animation active
    return true;
  else
    return false;
}

internal void
camera_move_animation(Game *game, Game_Draw *draw) {
  Animation *a = find_animation(animations, &camera.pose);

  add_keyframe(a, handle_camera_wait, NULL);

  if (a->keyframes.insert_index == 0)
    add_keyframe(a, camera.pose, camera.pose, 0.3f);

  Transform end = get_player_camera(-draw->degrees_between_players, game->active_player);
  add_keyframe(a, camera.pose, end, 0.5f, INTERP_SLERP);
}

internal void
pile_move_animation(Game *game, Game_Draw *draw, Pose *pile_pose, Vector3 offset, float32 omega) {
  Animation *a = find_animation(animations, pile_pose);
  Animation_Keyframe key = {};
  key.start = *pile_pose;
  key.time_duration = 0.5f;
  key.interpolation = INTERP_SLERP;
  key.end = get_pile_pose(game, draw, offset, omega);
  add_keyframe(a, &key);
}

bool8 handle_discard_animation_cleanup(void *args) {
  Card_Entity *e = (Card_Entity *)args;
  game_draw.discard_pile_entity->index = e->index;
  game_draw.card_entities.close(e);
  return true;
}

internal void
animation_discard(Animation *a, Card_Entity *e) {
  add_dynamic_keyframe(a, e->transform, &game_draw.discard_pile_entity->transform, 0.25f);

  Animation_Keyframe key = {};
  key.interpolation = INTERP_FUNC;
  key.func = handle_discard_animation_cleanup;
  key.func_args = (void *)e;
  add_keyframe(a, &key);
}

internal void
animation_discard_select_card(Player_Card *card) {
  Animation *a = 0;
  if (!card->flipped) {
    card->flipped = true;
    a = flip_card_animation(card);
  } else {
    a = find_animation(animations, &card->entity->transform);
  }

  Transform middle = card->entity->transform;
  middle.y += 2.0f;
  middle.yaw = 0.0f;
  add_keyframe(a, card->entity->transform, middle, 0.25f);

  animation_discard(a, card->entity);
}

internal void
animation_place_picked_up(Player_Card *card) {
  Animation *a = find_animation(animations, &game_draw.picked_up_card_entity->transform);
  Transform picked_up_dest = card->entity->transform;
  picked_up_dest.yaw = 0.0f;
  picked_up_dest.y = 0.0f;
  add_keyframe(a, game_draw.picked_up_card_entity->transform, picked_up_dest, 0.5f);
  card->entity = game_draw.picked_up_card_entity;
  game_draw.picked_up_card_entity = 0;
}

bool8 handle_animation_pause(void *args) {
  Card_Entity *e = (Card_Entity *)args;
  if (e->hovered)
    return false;
  else
    return true;
}

internal void
animation_lift_hovered(Card_Entity *e) {
  Animation *a = find_animation(animations, &e->transform);
  if (a->keyframes.insert_index == 0) {
    Transform bottom = e->transform;
    bottom.y = 0.0f;
    Transform top = e->transform;
    top.y += 0.2f;
    add_keyframe(a, bottom, top, 0.03f);
    add_keyframe(a, handle_animation_pause, (void *)e);
    add_keyframe(a, top, bottom, 0.03f);
  }
}