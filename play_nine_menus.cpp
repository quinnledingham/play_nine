internal void
menu_set_input(Menu *menu, Menu_Input *input) {
    input->hot = menu->hot_section;
    input->hot_ptr = &menu->hot_section;
}

// returns game mode
internal s32
draw_main_menu(State *state, Menu *menu, Menu_Input *input, Vector2_s32 window_dim) {
    //menu_update_active(&state->menu_list.main.active, 0, 2, state->controller.backward,  state->controller.forward);

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.6f, 0.6f);

    if (!menu->initialized) {
        menu->initialized = true;
    
        u32 buttons_count = 3;    
        menu->sections = { 1, 7 };
        menu->interact_region[0] = { 0, 2 };
        menu->interact_region[1] = menu->sections;
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    menu_text(menu, "play_nine", { 231, 213, 36,  1 }, { 0, 0 }, { 1, 2 }); 

    if (menu_button(menu, "Local", *input, { 0, 2 }, { 1, 1 })) {
        state->menu_list.mode = LOCAL_MENU;
        state->game.num_of_players = 1;
        default_player_name_string(state->game.players[0].name, 0);
    }
    if (menu_button(menu, "Test", *input, { 0, 3 }, { 1, 1 })) {
        state->game = get_test_game();
        state->menu_list.mode = SCOREBOARD_MENU;    
    }
    if (menu_button(menu, "Host Online", *input, { 0, 4 }, { 1, 1 })) {
        state->menu_list.mode = HOST_MENU;
    }
    if (menu_button(menu, "Join Online", *input, { 0, 5 }, { 1, 1 })) {
        state->menu_list.mode = JOIN_MENU;
    }
    if (menu_button(menu, "Quit", *input, { 0, 6 }, { 1, 1 })) 
        return true;

    draw_card_bitmaps(card_bitmaps, window_dim);

    return false;
}

internal s32
draw_local_menu(State *state, Menu *menu, Menu_Input *input, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.9f, 0.9f);

    if (!menu->initialized) {
        menu->initialized = true;

        menu->sections = { 2, 9 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 2, 9 };
    }

    menu_set_input(menu, input);

    state->game_draw.degrees_between_players = 360.0f / float32(state->game.num_of_players);
    state->game_draw.radius = get_draw_radius(game->num_of_players, hand_width, 3.2f);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );
    draw_rect(menu->rect.coords, 0, menu->rect.dim, { 0, 0, 0, 0.2f} );

    s32 menu_row = 0;
    for (menu_row; menu_row < (s32)game->num_of_players - 1; menu_row++) {
        if (menu_textbox(menu, game->players[menu_row].name, *input, { 0, menu_row }, { 2, 1 }) && menu_row == state->client_game_index) {
            client_set_name(state->client, game->players[menu_row].name);
        }
    }

    if (!state->is_client) {
        if (menu_textbox(menu, game->players[menu_row].name, *input, { 0, menu_row }, { 2, 7 - (s32)game->num_of_players }, { 2, 1 }) && menu_row == state->client_game_index) {
            client_set_name(state->client, game->players[menu_row].name);
        }
        menu_row = 6;

        if (menu_button(menu, "+ Player", *input, { 0, menu_row }, { 1, 1 })) {
            if (game->num_of_players != 6) {
                default_player_name_string(game->players[game->num_of_players].name, game->num_of_players);
                game->num_of_players++;
            }
        }
        if (menu_button(menu, "- Player", *input, { 1, menu_row }, { 1, 1 })) {
            if (game->num_of_players != 1) {
                game->num_of_players--;
            }
        }

        if (menu_button(menu, "+ Bot", *input, { 0, menu_row + 1 }, { 1, 1 })) {
            
        }
        if (menu_button(menu, "- Bot", *input, { 1, menu_row + 1 }, { 1, 1 })) {
            
        }

        if (menu_button(menu, "Start", *input, { 0, menu_row + 2 }, { 1, 1 })) {
            if (game->num_of_players != 1) {
                state->menu_list.mode = IN_GAME;
                load_name_plates(&state->game, &state->game_draw);
                start_game(&state->game, game->num_of_players);
            } else {
                add_onscreen_notification(&state->notifications, "Not enough players");
            }
        }
    } else {
        if (menu_textbox(menu, game->players[menu_row].name, *input, { 0, menu_row }, { 2, 9 - (s32)game->num_of_players }, { 2, 1 }) && menu_row == state->client_game_index) {
            client_set_name(state->client, game->players[menu_row].name);
        }
        menu_row = 6;
    }

    s32 back_width = 1;
    if (state->is_client)
        back_width = 2;

    if (menu_button(menu, "Back", *input, { 1, menu_row + 2 }, { back_width, 1 })) {
        state->menu_list.mode = MAIN_MENU;
        game->num_of_players = 1;
        menu->hot_section = { 0, 0 };

        if (state->is_server) {
            state->is_server = false;
            close_server();
        } else if (state->is_client) {
            state->is_client = false;
            client_close_connection(state->client);
        }
    }

    return false;
}

internal s32
draw_pause_menu(State *state, Menu *menu, Menu_Input *input, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect   = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    if (!menu->initialized) {
        menu->initialized = true;

        menu->sections = { 1, 3 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 0, 0 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 0, 0, 0, 0.5f} );

    if (menu_button(menu, "Quit Game", *input, { 0, 0 }, { 1, 1 })) {
        state->menu_list.mode = MAIN_MENU;
        game->num_of_players = 1;
        menu->hot_section = { 0, 0 };
        reset_game(game);
        if (state->is_server) {
            win32_release_mutex(state->mutex);
            close_server();
        }
    }

    return false;
}

/*
0 do nothing
1 next hole
2 quit game
*/ 
internal s32
draw_scoreboard(Menu *menu, Game *game, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.9f, 0.9f);

    s32 scroll_length = 8;

    if (!menu->initialized) {
        menu->initialized = true;

        menu->sections = { (s32)game->num_of_players + 1, scroll_length + 3 };
        menu->interact_region[0] = { 0, scroll_length + 2 };
        menu->interact_region[1] = { (s32)game->num_of_players + 2, scroll_length + 2 };

        menu->hot_section = menu->interact_region[0];

        if ((s32)game->holes_played <= scroll_length)
            menu->scroll = { 1, (s32)game->holes_played };
        else
            menu->scroll = { (s32)game->holes_played - scroll_length + 1, (s32)game->holes_played + 1 };
    }

    menu_set_input(menu, input);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );
    draw_rect(menu->rect.coords, 0, menu->rect.dim, { 0, 0, 0, 0.2f} );

    // columns
    for (s32 i = 0; i < menu->sections.x; i++) {
        if (i % 2 == 0) {
            menu_rect(menu, {i, 0}, { 1, menu->sections.y}, { 0, 0, 0, 0.2f});
        }
    }

    menu_rect(menu, {0, 0}, { menu->sections.x, 1}, { 255, 255, 0, 0.01f});
    menu_rect(menu, {0, menu->sections.y - 2}, { menu->sections.x, 1}, { 255, 255, 0, 0.01f});

    // Top
    Vector4 text_color = { 231, 213, 36,  1 };

    menu_text(menu, "Hole", text_color, { 0, 0 }, { 1, 1 }); 
    for (u32 i = 0; i < game->num_of_players; i++) {
        menu_text(menu, game->players[i].name, text_color, { (s32)i + 1, 0 }, { 1, 1 }); 
    }

    char *subtotal_text = "subtotal";
    char hole_text[3];
    char score_text[4]; // can't get over 99 on a hole. 12 * 8 = 96 BUT -30 is the lowsest score so need 3 digits
    ASSERT(MAX_HOLES < 99);

    s32 index = 0;
    for (s32 i = 0; i < (s32)game->holes_played; i++) {
        if (i + 1 < menu->scroll.x || i + 1 > menu->scroll.y) {
            continue;
        }

        platform_memory_set(hole_text, 0, 3);
        s32_to_char_array(hole_text, 3, i + 1);
        menu_text(menu, hole_text, text_color, { 0, index + 1 }, { 1, 1 });

        for (s32 player_index = 0; player_index < (s32)game->num_of_players; player_index++) {
            platform_memory_set(score_text, 0, 4);
            s32_to_char_array(score_text, 4, game->players[player_index].scores[i]);
            menu_text(menu, score_text, text_color, { player_index + 1, index + 1 }, { 1, 1 }); 
        }

        index++;
    }

    char total_text[4]; // max score = 12 * 8 * 9 = 864
    menu_text(menu, "Total", text_color, { 0, scroll_length + 1 }, { 1, 1 });
    for (s32 player_index = 0; player_index < (s32)game->num_of_players; player_index++) {
        u32 player_total = 0;
        for (u32 i = 0; i < game->holes_played; i++)
            player_total += game->players[player_index].scores[i];
        platform_memory_set(total_text, 0, 4);
        s32_to_char_array(total_text, 4, player_total);
        menu_text(menu, total_text, text_color, { player_index + 1, scroll_length + 1 }, { 1, 1 }); 
    }

    const char *play_button_text;
    if (!game->game_over)
        play_button_text = "Next Hole";
    else
        play_button_text = "Play Again";

    if (menu_button(menu, play_button_text, *input, { 0, scroll_length + 2 }, { (s32)game->num_of_players - 1, 1 })) {
        if (!game->game_over)
            start_hole(game);
        else
            start_game(game, game->num_of_players);
        return 1;
    }
    if (menu_button(menu, "Back", *input, { (s32)game->num_of_players - 1, scroll_length + 2 }, { 1, 1 })) {
        return 1;
    }
    if (menu_button(menu, "Quit", *input, { (s32)game->num_of_players, scroll_length + 2 }, { 1, 1 })) {
        return 2;   
    }

    return 0;
}

internal void
draw_host_menu(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    if (!menu->initialized) {
        menu->initialized = true;
    
        menu->sections = { 2, 3 };
        menu->interact_region[0] = { 0, 1 };
        menu->interact_region[1] = { 2, 3 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );

    menu_text(menu, "Enter Port:", { 231, 213, 36,  1 }, { 0, 0 }, { 2, 1 }); 
    
    if (menu_textbox(menu, state->port, *input, { 0, 1 }, { 2, 1 })) {

    }

    if (menu_button(menu, "Host", *input, { 0, 2 }, { 1, 1 })) {
        DWORD thread_id;
        state->client_game_index = 0;
        online.server_handle = CreateThread(0, 0, play_nine_server, (void*)state, 0, &thread_id);
    }
    if (menu_button(menu, "Back", *input, { 1, 2 }, { 1, 1 })) {
        state->menu_list.mode = MAIN_MENU;
    }
}

internal void
draw_join_menu(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    if (!menu->initialized) {
        menu->initialized = true;
    
        menu->sections = { 2, 5 };
        menu->interact_region[0] = { 1, 0 };
        menu->interact_region[1] = { 2, 5 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );

    Vector4 text_color = { 231, 213, 36,  1 };

    menu_text(menu, "Name:", text_color, { 0, 0 }, { 1, 1 }); 
    menu_text(menu, "Ip:",   text_color, { 0, 1 }, { 1, 1 }); 
    menu_text(menu, "Port:", text_color, { 0, 2 }, { 1, 1 }); 
    
    menu_textbox(menu, state->name, *input, { 1, 0 }, { 1, 1 });
    menu_textbox(menu, state->ip,   *input, { 1, 1 }, { 1, 1 });
    menu_textbox(menu, state->port, *input, { 1, 2 }, { 1, 1 });

    if (menu_button(menu, "Join", *input, { 1, 3 }, { 1, 1 })) {
        if (qsock_client(&state->client, state->ip, state->port, TCP)) {
            state->menu_list.mode = LOCAL_MENU;
            state->is_client = true;

            client_set_name(state->client, state->name);
        } else {
            add_onscreen_notification(&state->notifications, "Unable to join");
        }
    }
    if (menu_button(menu, "Back", *input, { 1, 4 }, { 1, 1 })) {
        state->menu_list.mode = MAIN_MENU;
    }
}