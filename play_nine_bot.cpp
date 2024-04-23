inline bool8
is_pair(u32 *cards, bool8 *flipped, u32 index) {
  if (index < 4) {
    if (deck[cards[index]] == deck[cards[index + 4]] && flipped[index + 4]) 
      return true;
  } else {
    if (deck[cards[index]] == deck[cards[index - 4]] && flipped[index - 4])
      return true;
  }

  return false;
}

// just pick first not flipped if random doesnt immediately return a result
inline u32
random_not_flipped_card_index(bool8 *flipped) {
  u32 index = 0;
  u32 attempts = 0;
  
  do {
    attempts++;
    if (attempts > 3) {
      for (u32 i = 0; i < HAND_SIZE; i++) {
        if (!flipped[i])
          return i;    
      }
    }
    
    index = random(0, 8);  
  } while(flipped[index]);

  return index;
}

internal void
do_bot_selected_update(bool8 selected[SELECTED_SIZE], Game *game, float32 *bot_thinking_time, float64 frame_time_s) {

  *bot_thinking_time += (float32)frame_time_s;
  if (game->bot_thinking_time < 2.0f)
    return;

  *bot_thinking_time = 1.5f;

  Player *active_player = &game->players[game->active_player];
  
  switch(game->turn_stage) {
    case FLIP_CARD: {
      
      if (get_number_flipped(active_player->flipped) == HAND_SIZE - 1 && get_score(active_player->cards) > 15) {
        selected[PASS_BUTTON] = true;
        return;
      }

      u32 index = random_not_flipped_card_index(active_player->flipped);
      selected[index] = true;
    } break;

    case SELECT_PILE: {
      s8 discard_number = deck[game->discard_pile[game->top_of_discard_pile - 1]];
      
      // Match discard with number
      for (u32 i = 0; i < HAND_SIZE; i++) {
        if (!active_player->flipped[i]) continue;
        
        if (discard_number == deck[active_player->cards[i]] && !is_pair(active_player->cards, active_player->flipped, i)) {
          selected[DISCARD_PILE] = true;
          return;
        }
      }
              
      // pick discard if it is low
      if (discard_number < 6)
        selected[DISCARD_PILE] = true;
      else
        selected[PICKUP_PILE] = true;
    } break;

    case SELECT_CARD: {
      s8 new_card = deck[game->new_card];
      
      // Match for pairs
      for (u32 i = 0; i < HAND_SIZE; i++) {
        if (!active_player->flipped[i]) continue;
        
        if (new_card == deck[active_player->cards[i]] && !is_pair(active_player->cards, active_player->flipped, i)) {
          if      (i <  4) selected[i + 4] = true;
          else if (i >= 4) selected[i - 4] = true;
          return;
        }
      }

      // @TODO for next two checks look ahead to see if you would give
      // the next player a pair.
      
      // if from pickup pile discard if high card
      if (game->pile_card) {
        if (new_card > 8) {
          selected[DISCARD_PILE] = true;
          return;
        }
      }

      // If good look for where to place it
      for (u32 i = 0; i < HAND_SIZE; i++) {
        if (!active_player->flipped[i]) continue;
        
        if (new_card < deck[active_player->cards[i]] && 4 < deck[active_player->cards[i]] && !is_pair(active_player->cards, active_player->flipped, i)) {
          selected[i] = true;
          return;
        }
      }

      // Lastly replace run flipped card
      for (u32 i = 0; i < HAND_SIZE; i++) {
        if (!active_player->flipped[i]) {
          selected[i] = true;
          return;
        }
      }
      
      u32 index = random(0, 8);
      selected[index] = true;
    } break;
  }
}

