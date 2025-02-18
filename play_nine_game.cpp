internal void
increment_player(u32 *counter, u32 num_of_players) {
    (*counter)++;
    if (*counter >= num_of_players)
        (*counter) = 0;
}

internal void
decrement_player(u32 *counter, u32 num_of_players) {
    (*counter)--;
    if (*counter < 0)
        (*counter) = num_of_players - 1;
}

//
// in-game
//

internal void
next_player(Game *game) {
    // Flip all cards after final turn
    if (game->round_type == FINAL_ROUND) {
        game->last_turn++;
        for (u32 i = 0; i < HAND_SIZE; i++) {
            if (!game->players[game->active_player].flipped[i]) {
                game->players[game->active_player].flipped[i] = true;
                add_draw_signal(draw_signals, SIGNAL_FLIP_CARD, i, game->active_player);
            }
        }
    }

    // END HOLE
    if (game->last_turn == game->num_of_players && game->round_type != HOLE_OVER) {
        update_scores(game);
        game->round_type = HOLE_OVER;
        game->turn_stage = FLIP_CARD;

        increment_player(&game->starting_player, game->num_of_players);

        // END GAME
        game->holes_played++;
        if (game->holes_played == game->holes_length) {
            game->game_over = true;
        }

        fill_total_scores(game);
        return; // Don't need to move ahead a player now
    }

    game->pile_card = false;
    game->turn_stage = SELECT_PILE;
    increment_player(&game->active_player, game->num_of_players);

    if (game->active_player == game->starting_player && game->round_type == FLIP_ROUND) {
        game->round_type = REGULAR_ROUND;
    }

    if (game->round_type == FLIP_ROUND) {
        game->turn_stage = FLIP_CARD;
    }

    if (game->players[game->active_player].is_bot)
        game->bot_thinking_time = 0.0f;

    game->turn_time = 0.0f;

    add_draw_signal(draw_signals, SIGNAL_NEXT_PLAYER_ROTATION);
}

internal void
do_update_with_input(Game *game, bool8 input[GI_SIZE]) {
    Player *active_player = &game->players[game->active_player];
    switch(game->turn_stage) {
        case SELECT_PILE: {
            if (input[GI_PICKUP_PILE]) {
                add_draw_signal(draw_signals, SIGNAL_NEW_CARD_FROM_PILE);
                game->new_card = game->pile[game->top_of_pile++];

                // @SPECIAL case
                // Probably shouldn't happen in a real game
                if (game->top_of_pile + game->num_of_players >= DECK_SIZE) {
                    game->round_type = FINAL_ROUND;
                }

                game->turn_stage = SELECT_CARD;
                game->pile_card = true;
            } else if (input[GI_DISCARD_PILE]) {
                add_draw_signal(draw_signals, SIGNAL_NEW_CARD_FROM_DISCARD);
                game->new_card = game->discard_pile[game->top_of_discard_pile - 1];
                game->top_of_discard_pile--;
                game->turn_stage = SELECT_CARD;
            }
        } break;

        case SELECT_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (input[i]) {
                    game->discard_pile[game->top_of_discard_pile++] = active_player->cards[i];
                    add_draw_signal(draw_signals, SIGNAL_REPLACE, i, game->active_player, active_player->flipped[i]);
                    active_player->flipped[i] = true;
                    active_player->cards[i] = game->new_card;
                    game->new_card = 0;

                    if (get_number_flipped(active_player->flipped) == HAND_SIZE)
                        game->round_type = FINAL_ROUND;
                    next_player(game);
                    return;
                }
            }

            if (game->pile_card && input[GI_DISCARD_PILE]) {
                game->discard_pile[game->top_of_discard_pile++] = game->new_card;
                game->new_card = 0;
                game->turn_stage = FLIP_CARD;
                add_draw_signal(draw_signals, SIGNAL_DISCARD_SELECTED);
            }
        } break;

        case FLIP_CARD: {
            for (u32 i = 0; i < HAND_SIZE; i++) {
                if (input[i] && !active_player->flipped[i]) {    
                    add_draw_signal(draw_signals, SIGNAL_FLIP_CARD, i, game->active_player);
                    active_player->flipped[i] = true;

                    if (game->round_type == FLIP_ROUND) {
                        // check if player turn over
                        if (get_number_flipped(active_player->flipped) == 2) {
                            next_player(game);
                            return;
                        }
                    } else {                
                        if (get_number_flipped(active_player->flipped) == HAND_SIZE && game->round_type != FINAL_ROUND) {
                            game->round_type = FINAL_ROUND; 
                            game->last_turn = 0;
                        }
                        next_player(game);
                        return;
                    }
                }

                if (input[GI_PASS_BUTTON] && get_number_flipped(active_player->flipped) == HAND_SIZE - 1) {
                    next_player(game);
                    return;
                }

            }
        } break;
    }
}
