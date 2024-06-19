
internal void
default_player_name_string(char buffer[MAX_NAME_SIZE], u32 number) {
    platform_memory_set((void*)buffer, 0, MAX_NAME_SIZE);
    buffer[8] = 0;
    memcpy(buffer, "Player ", 7);
    buffer[7] = number + 48; // + 48 turns single digit number into ascii value
}

internal void
default_bot_name_string(char buffer[MAX_NAME_SIZE], u32 number) {
    platform_memory_set((void*)buffer, 0, MAX_NAME_SIZE);
    buffer[5] = 0;
    memcpy(buffer, "Bot ", 4);
    buffer[4] = number + 48; // + 48 turns single digit number into ascii value
}
/*
n_o_p = 5
index = 2

0 1 2 3 4

0 1 3 4

0 1 2 3
*/

internal void
add_player(Game *game, bool8 bot) {
    if (game->num_of_players != MAX_PLAYERS) {
        // count non bots
        u32 num_of_players = 1;
        u32 num_of_bots = 1;
        for (u32 i = 0; i < game->num_of_players; i++) {
            if (!game->players[i].is_bot) 
                num_of_players++;
            else
                num_of_bots++;
        }
        if (!bot)
            default_player_name_string(game->players[game->num_of_players].name, num_of_players);
        else
            default_bot_name_string(game->players[game->num_of_players].name, num_of_bots);

        game->players[game->num_of_players].is_bot = bot;
        game->num_of_players++;
    }
}

internal void
remove_online_player(Game *game, u32 index) {
    for (u32 i = 0; i < 5; i++) {
        if (online.players[i].in_use && online.players[i].game_index > index)
            online.players[i].game_index--;
    }
}

internal void
remove_player_index(Game *game, u32 index) {
    u32 dest_index = index;
    u32 src_index = index + 1;
    for (src_index; src_index < game->num_of_players; src_index++) {        
        game->players[dest_index++] = game->players[src_index];
    }
    game->num_of_players--;
}

// removes last added - used on menu
internal s32
remove_player(Game *game, bool8 bot) {
    for (u32 i = game->num_of_players - 1; i > 0; i--) {
        if ((!game->players[i].is_bot && !bot) || (game->players[i].is_bot && bot)) {
            remove_player_index(game, i);
            return i;
        }
    }

    return -1;
}

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

internal void
init_deck() {
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

    // init hand coords
    float32 card_width = 2.0f;
    float32 card_height = 3.2f;
    float32 padding = 0.2f;
    hand_width = card_width * 4.0f + padding * 3.0f;

    u32 card_index = 0;
    for (s32 y = 1; y >= 0; y--) {
        for (s32 x = -2; x <= 1; x++) {
            hand_coords[card_index++] = { 
                float32(x) * card_width  + (float32(x) * padding) + (card_width / 2.0f)  + (padding / 2.0f), 
                float32(y) * card_height + (float32(y) * padding) - (card_height / 2.0f) - (padding / 2.0f)
            };
        }
    }
}

// srand at beginning of main_loop()
internal s32
random(s32 lower, s32 upper) {
    return lower + (rand() % (upper - lower));
}

internal void
shuffle_pile(u32 *pile) {
    for (u32 i = 0; i < DECK_SIZE; i++) {
        bool8 not_new_card = true;
        while(not_new_card) {
            pile[i] = random(0, DECK_SIZE);
            not_new_card = false;
            for (u32 j = 0; j < i; j++) {
                if (pile[i] == pile[j])
                    not_new_card = true;
            }
        }
    }
}

internal void
deal_cards(Game *game) {
     for (u32 i = 0; i < game->num_of_players; i++) {
         for (u32 card_index = 0; card_index < HAND_SIZE; card_index++) {
            game->players[i].cards[card_index] = game->pile[game->top_of_pile++];
            game->players[i].flipped[card_index] = false;
         }
     }

    game->discard_pile[game->top_of_discard_pile++] = game->pile[game->top_of_pile++];
}

internal void
start_hole(Game *game) {
    game->top_of_pile = 0;
    game->top_of_discard_pile = 0;
    shuffle_pile(game->pile);
    deal_cards(game);
    game->active_player = game->starting_player;
    game->round_type = FLIP_ROUND;
    game->turn_stage = FLIP_CARD;
    game->last_turn = 0;
    game->turn_time = 0.0f;
}

internal void
start_game(Game *game, u32 num_of_players) {
    game->holes_played = 0;
    game->starting_player = 0;
    game->num_of_players = num_of_players;
    game->game_over = false;
    start_hole(game);
}

internal Game
get_test_game() {
    Game game = {};
    game.num_of_players = 6;
    game.holes_played = 4;
    game.round_type = HOLE_OVER;

    for (u32 i = 0; i < game.num_of_players; i++) {
        default_player_name_string(game.players[i].name, i);
        game.players[i].scores[0] = i;
    }

    game.players[0].scores[0] = -10;

    fill_total_scores(&game);
    
    return game;
}
