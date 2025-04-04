void init_deck() {
  u32 deck_index = 0;
  for (s32 i = 0; i <= 12; i++) {
    for (u32 j = deck_index; j < deck_index + 8; j++) {
      deck[j] = i;
    }
    deck_index += 8;
  }

  for (u32 j = deck_index; j < deck_index + 4; j++) {
    deck[j] = -5;
  }
}

// srand at beginning of main_loop()
internal s32
random(s32 lower, s32 upper) {
    //return lower + (rand() % (upper - lower));
    s32 possible = upper - lower;
    return SDL_rand(possible) + lower;
}

// assumes that you want to shuffle a pile of size DECK_SIZE
void set_pile_to_random_deck(Card_Pile *pile) {
  for (u32 i = 0; i < DECK_SIZE; i++) {
    bool8 not_new_card = true;
    u32 index = 0;
    while(not_new_card) {
      index = random(0, DECK_SIZE);

      // check if this card has already been added to the pile
      not_new_card = false;
      for (u32 j = 0; j < i; j++) {
        if (index == pile->indices[j])
          not_new_card = true;
      }
    }

    pile->add_card(index);
  }
}

void deal_cards(Game *game) {
  for (u32 player_index = 0; player_index < game->players_count; player_index++) {
    for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
      game->players[player_index].cards[card_index].index = game->draw_pile.draw_card();
    }
  }

  game->discard_pile.add_card(game->draw_pile.draw_card());
}

void start_hole(Game *game) {
  game->draw_pile.reset();
  game->discard_pile.reset();
  set_pile_to_random_deck(&game->draw_pile);
  deal_cards(game);

  game->active_player = game->starting_player;
  game->round_type = FLIP_ROUND;
  game->turn.stage = FLIP_CARD;
}

void start_game(Game *game, u32 players_count) {
  *game = {};
  game->holes_played = 0;
  game->starting_player = 0;
  game->players_count = players_count;
  game->game_over = false;

  start_hole(game);
}
/*
void draw_game_row(Game *game, u8 row) {
  u32 start_card_index = 0;
  if (row != 0)
    start_card_index = 4;

  for (u32 player_index = 0; player_index < game->players_count; player_index++) {
    for (u32 card_index = start_card_index; card_index < start_card_index + 4; card_index++) {
      if (game->players[player_index].cards[card_index].flipped)
        print("%3d ", deck[game->players[player_index].cards[card_index].index]);
      else
        print("||| ");
    }
    print("  ");
  }
}

void draw_game(Game *game) {
  print("\n%d %d    ", deck[game->draw_pile.top_card()], deck[game->discard_pile.top_card()]);
  print("%d\n", deck[game->turn.picked_up_card]);

  draw_game_row(game, 0);
  print("\n");
  draw_game_row(game, 1);
  print("\n");
  print("Active Player: %d\n", game->active_player);
}
*/

// returns so that the animation can be added on to
internal Animation*
flip_card_animation(u32 p_i, u32 c_i) {
  float32 time_duration = 0.3f;

  Pose *pose = &game_draw.cards[p_i][c_i];
  Animation *a = find_animation(animations, pose);

  Pose middle = *pose;
  middle.y += 1.0f;
  middle.yaw += PI/2;

  add_keyframe(a, *pose, middle, time_duration/2.0f);

  Pose final = *pose;
  final.yaw += PI;

  add_keyframe(a, *pose, final, time_duration/2.0f);

  return a;
}

u32 get_number_flipped(Player_Card *cards) {
    u32 number_flipped = 0;
    for (u32 i = 0; i < HAND_SIZE; i++) {
        if (cards[i].flipped) {
            number_flipped++;
        }
    }
    return number_flipped;
}

void flip_cards(u32 p_i, Player_Card *cards) {
  for (u32 i = 0; i < HAND_SIZE; i++) {
    if (!cards[i].flipped) {
      cards[i].flipped = true;
      flip_card_animation(p_i, i);
    }
  }
}

internal void
increment(u8 *counter, u32 max) {
    (*counter)++;
    if (*counter >= max)
        (*counter) = 0;
}

internal void
decrement(u8 *counter, u32 max) {
    (*counter)--;
    if (*counter < 0)
        (*counter) = max - 1;
}

internal void
camera_move_animation(Game *game, Game_Draw *draw) {
  Animation *a = find_animation(animations, &camera.pose);
  
  if (a->keyframes.insert_index == 0)
    add_keyframe(a, camera.pose, camera.pose, 0.3f);

  Animation_Keyframe key = {};
  key.start = camera.pose;
  key.time_duration = 0.5f;
  key.interpolation = INTERP_SLERP;
  key.end = get_player_camera(-draw->degrees_between_players, game->active_player);
  add_keyframe(a, &key);
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

internal void
next_player(Game *game, Player *active_player) {
  // Flip all cards after final turn
  if (game->round_type == FINAL_ROUND) {
    game->last_turn++;
    flip_cards(game->active_player, active_player->cards);
  }

  // End Hole
  if (game->last_turn == game->players_count && game->round_type != HOLE_OVER) {
    game->round_type = HOLE_OVER;
    game->turn.stage = FLIP_CARD;

    increment(&game->starting_player, game->players_count);

    // End Game
    game->holes_played++;
    if (game->holes_played == game->holes_length) {
      game->game_over = true;
    }

    return;
  }

  game->turn = {};
  increment(&game->active_player, game->players_count);

  if (game->round_type == FLIP_ROUND) {
    if (game->active_player == game->starting_player) {
      game->round_type = REGULAR_ROUND;
    } else {
      game->turn.stage = FLIP_CARD;
    }
  }

  camera_move_animation(game, &game_draw);
  
  pile_move_animation(game, &game_draw, &game_draw.draw_pile_pose,      game_draw.draw_pile_offset,        PI);
  pile_move_animation(game, &game_draw, &game_draw.discard_pile_pose,   game_draw.discard_pile_offset,   0.0f);
  pile_move_animation(game, &game_draw, &game_draw.picked_up_card_pose, game_draw.picked_up_card_offset, 0.0f);

}

internal void
update_game_select_pile(Game *game, bool8 input[GI_SIZE], Player *active_player) {
  if (input[GI_DRAW_PILE]) {
    game->turn.picked_up_card = game->draw_pile.draw_card();

    // @SPECIAL case
    // Probably shouldn't happen in a real game
    //if (game->draw_pile.top + game->players_count >= DECK_SIZE) {
    //    game->round_type = FINAL_ROUND;
    //}
    
    game->turn.new_card = true;
    game->turn.stage = SELECT_CARD;
  } else if (input[GI_DISCARD_PILE]) {
    game->turn.picked_up_card = game->discard_pile.draw_card();
    game->turn.new_card = false;
    game->turn.stage = SELECT_CARD;
  }

}

void update_game_select_card(Game *game, bool8 input[GI_SIZE], Player *active_player) {
  for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
    // card not clicked
    if (!input[card_index]) {
      continue;
    }

    // card clicked
    Player_Card *card = &active_player->cards[card_index];
    Animation *a = 0;
    if (!card->flipped) {
      card->flipped = true;
      a = flip_card_animation(game->active_player, card_index);
    } else {
      a = find_animation(animations, &game_draw.cards[game->active_player][card_index]);
    }

    add_dynamic_keyframe(a, game_draw.cards[game->active_player][card_index], &game_draw.discard_pile_pose, 0.5f);

    game->discard_pile.add_card(active_player->replace_card(card_index, game->turn.picked_up_card));

    if (get_number_flipped(active_player->cards) == HAND_SIZE) {
      game->round_type = FINAL_ROUND;
    }

    next_player(game, active_player);

    return;
  }

  if (game->turn.new_card && input[GI_DISCARD_PILE]) {
    game->discard_pile.add_card(game->turn.picked_up_card);
    game->turn.stage = FLIP_CARD;
    game->turn.picked_up_card = -1;
  }
}

internal void 
update_game_flip_card(Game *game, bool8 input[GI_SIZE], Player *active_player) {
  for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
    if (input[card_index] && !active_player->cards[card_index].flipped) {
      active_player->cards[card_index].flipped = true;

      flip_card_animation(game->active_player, card_index);

      if (game->round_type == FLIP_ROUND) {
        if (get_number_flipped(active_player->cards) == 2) {
          next_player(game, active_player);
          return;
        }
      } else {
        if (get_number_flipped(active_player->cards) == HAND_SIZE && game->round_type != FINAL_ROUND) {
          game->round_type = FINAL_ROUND;
          game->last_turn = 0;
        }
        next_player(game, active_player);
        return;
      }
    }
  }

  if (input[GI_PASS_BUTTON] && get_number_flipped(active_player->cards) == HAND_SIZE - 1) {
    next_player(game, active_player);
    return;
  }
}

internal void 
update_game_with_input(Game *game, bool8 input[GI_SIZE]) {
  Player *active_player = &game->players[game->active_player];
  switch(game->turn.stage) {
    case SELECT_PILE:
      update_game_select_pile(game, input, active_player);
      break;
    case SELECT_CARD:
      update_game_select_card(game, input, active_player);
      break;
    case FLIP_CARD:
      update_game_flip_card(game, input, active_player);
      break;
  }
}