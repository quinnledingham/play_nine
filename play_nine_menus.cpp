internal void
menu_set_input(Menu *menu, Menu_Input *input) {
    input->hot = menu->hot_section;
    input->hot_ptr = &menu->hot_section;
}

internal void
quit_to_main_menu(State *state, Menu *menu) {
    os_wait_mutex(state->mutex);

    state->menu_list.mode = MAIN_MENU;
    state->game.num_of_players = 1;
    menu->hot_section = { 0, 0 };

    if (state->is_server) {
        state->is_server = false;
        os_release_mutex(state->mutex);
        close_server();
    } else if (state->is_client) {
        state->is_client = false;
        client_close_connection(state->client);
    }

    os_release_mutex(state->mutex);
}

// returns game mode
internal s32
draw_main_menu(State *state, Menu *menu, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.6f, 0.6f);

    if (!menu->initialized) {
        menu->initialized = true;
        menu->sections = { 1, 7 };
        menu->interact_region[0] = { 0, 2 };
        menu->interact_region[1] = menu->sections;
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    menu->gui.start();
    
    menu_text(menu, "play_nine", { 231, 213, 36,  1 }, { 0, 0 }, { 1, 2 }); 

    if (menu_button(menu, "Local", *input, { 0, 2 }, { 1, 1 })) {
        state->menu_list.mode = LOCAL_MENU;
        state->game.num_of_players = 1;
        default_player_name_string(state->game.players[0].name, 1);
    }
    if (menu_button(menu, "Host Online", *input, { 0, 3 }, { 1, 1 })) {
        state->menu_list.mode = HOST_MENU;
    }
    if (menu_button(menu, "Join Online", *input, { 0, 4 }, { 1, 1 })) {
        state->menu_list.mode = JOIN_MENU;
    }
    if (menu_button(menu, "Settings", *input, { 0, 5 }, { 1, 1 })) {
        state->menu_list.previous_mode = MAIN_MENU;
        state->menu_list.mode = SETTINGS_MENU;
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
    bool8 full_menu = (!state->is_client && !state->is_server) || state->is_server;

    if (!menu->initialized) {
        menu->initialized = true;

        menu->sections = { 2, 9 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 2, 9 };
    }

    menu_set_input(menu, input);
    menu->gui.start();

    state->game_draw.degrees_between_players = 360.0f / float32(state->game.num_of_players);
    state->game_draw.radius = get_draw_radius(game->num_of_players, hand_width, 3.2f);

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );
    draw_rect(menu->rect.coords, 0, menu->rect.dim, { 0, 0, 0, 0.2f} );

    s32 menu_row = 0;
    for (menu_row; menu_row < (s32)game->num_of_players; menu_row++) {
        Vector2_s32 section_coords = { 0, menu_row };
        Vector2_s32 section_dim = { 2, 1 };
        Vector2_s32 draw_dim = section_dim;
        
        if (menu_row == game->num_of_players - 1) {
            section_dim = { 2, 7 - (s32)game->num_of_players };
            if (state->is_client)
                section_dim.y += 2;
            else if (state->is_server)
                section_dim.y += 1;
        }
        
        if (menu_textbox(menu, game->players[menu_row].name, *input, section_coords, section_dim, draw_dim) && menu_row == state->client_game_index) {
            if (state->is_client)
                client_set_name(state->client, game->players[menu_row].name);
            else if (state->is_server)
                server_send_game(&state->game);            
        }

        if (game->players[menu_row].is_bot) {
            // Draw bot icon
            Vector2 coords = get_screen_coords(menu, section_coords);
            Vector2 dim = get_screen_dim(menu, draw_dim);
            Vector2 bot_dim = dim * 0.6f;
            bot_dim.x = bot_dim.y;
            coords.x += (bot_dim.y / 2.0f);
            coords.y += (dim.y / 2.0f) - (bot_dim.y / 2.0f);

            render_bind_pipeline(&shapes.text_pipeline);
            
            Descriptor v_color_set = render_get_descriptor_set(&layouts[4]);
            render_update_ubo(v_color_set, (void *)&play_nine_green);
            render_bind_descriptor_set(v_color_set);

            Object object = {};
            
            Descriptor desc = render_get_descriptor_set(&layouts[2]);
            object.index = render_set_bitmap(&desc, find_bitmap(&state->assets, "BOT"));
            render_bind_descriptor_set(desc);
            
            coords += bot_dim / 2.0f;
            Vector3 coords_v3 = { coords.x, coords.y, 0 };
            Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
            Vector3 dim_v3 = { bot_dim.width, bot_dim.height, 1 };
            object.model = create_transform_m4x4(coords_v3, rotation_quat, dim_v3);

            render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

            render_draw_mesh(&shapes.rect_mesh);
        }
    }
    
    menu_row = 6;

    if (full_menu) {
        if (!state->is_server) {
            if (menu_button(menu, "+ Player", *input, { 0, menu_row }, { 1, 1 })) {
                add_player(game, false);
            }
            if (menu_button(menu, "- Player", *input, { 1, menu_row }, { 1, 1 })) {
                remove_player(game, false);
            }
        } 

        if (menu_button(menu, "+ Bot", *input, { 0, menu_row + 1 }, { 1, 1 })) {
            add_player(game, true);
            server_send_game(&state->game);
        }
        if (menu_button(menu, "- Bot", *input, { 1, menu_row + 1 }, { 1, 1 })) {
            remove_player(game, true);
            server_send_game(&state->game);
        }

        if (menu_button(menu, "Start", *input, { 0, menu_row + 2 }, { 1, 1 })) {
            if (game->num_of_players != 1) {
                state->menu_list.mode = IN_GAME;
                start_game(&state->game, game->num_of_players);
                if (state->is_server) {
                    server_send_menu_mode(state->menu_list.mode);
                    server_send_game(&state->game);
                }
                add_draw_signal(draw_signals, SIGNAL_ALL_PLAYER_CARDS);
                add_draw_signal(draw_signals, SIGNAL_NAME_PLATES);
            } else {
                add_onscreen_notification(&state->notifications, "Not enough players");
            }
        }
    } 
    
    Vector2_s32 back_coords = { 1, menu_row + 2 };
    s32 back_width = 1;

    if (!full_menu) {
        back_coords = { 0, menu_row + 2 };
        back_width = 2;
    }
    
    if (menu_button(menu, "Back", *input, back_coords, { back_width, 1 })) {
        quit_to_main_menu(state, menu);
    }

    return false;
}

internal s32
draw_pause_menu(State *state, Menu *menu, Menu_Input *input, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect   = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.3f);

    if (!menu->initialized) {
        menu->initialized = true;

        menu->sections = { 1, 3 };
        if (input->full_menu) {
            menu->interact_region[0] = { 0, 0 };
            menu->interact_region[1] = { 1, 3 };
        } else {
            menu->interact_region[0] = { 0, 1 };
            menu->interact_region[1] = { 1, 3 };
        }
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    menu->gui.start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 0, 0, 0, 0.5f} );

    if (input->full_menu) {
        if (menu_button(menu, "Lobby", *input, { 0, 0 }, { 1, 1 })) {
            state->menu_list.mode = LOCAL_MENU;
            menu->hot_section = menu->interact_region[0];

            if (state->is_server)
                server_send_menu_mode(state->menu_list.mode);
        }
    }
    if (menu_button(menu, "Settings", *input, { 0, 1 }, { 1, 1 })) {
        state->menu_list.previous_mode = PAUSE_MENU;
        state->menu_list.mode = SETTINGS_MENU;
    }

    if (menu_button(menu, "Quit Game", *input, { 0, 2 }, { 1, 1 })) {
        quit_to_main_menu(state, menu);
    }

    return false;
}

// Hole 11
// buffer must be 8 bytes
internal void
get_hole_text(char *buffer, u32 hole) {
    platform_memory_set(buffer, 0, 8);
    platform_memory_copy(buffer, (void*)"Hole ", 5);
    s32_to_char_array(buffer + 5, 3, hole);
}

/*
0 do nothing
1 next hole
2 quit game
*/ 
internal s32
draw_scoreboard(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    Game *game = &state->game;
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
    menu->gui.start();

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
    if (input->full_menu) {
        if (!game->game_over)
            play_button_text = "Next Hole";
        else
            play_button_text = "Play Again";

        if (menu_button(menu, play_button_text, *input, { 0, scroll_length + 2 }, { (s32)game->num_of_players - 1, 1 })) {
            state->menu_list.mode = IN_GAME;
            if (!game->game_over)
                start_hole(game);
            else
                start_game(game, game->num_of_players);
            add_draw_signal(draw_signals, SIGNAL_ALL_PLAYER_CARDS);    

            if (state->is_server) {
                server_send_menu_mode(state->menu_list.mode);
                server_send_game(&state->game);
            }
        }
        if (menu_button(menu, "Back", *input, { (s32)game->num_of_players - 1, scroll_length + 2 }, { 1, 1 })) {
            state->menu_list.mode = IN_GAME;

            if (state->is_server) {
                server_send_menu_mode(state->menu_list.mode);
            }
        }
    }

    Vector2_s32 quit_coords = { (s32)game->num_of_players, scroll_length + 2 };
    s32 quit_width = 1;

    if (!input->full_menu) {
        quit_coords = { 0, scroll_length + 2 };
        quit_width = (s32)game->num_of_players + 1;
    }

    if (menu_button(menu, "Quit", *input, quit_coords, { quit_width, 1 })) {
        quit_to_main_menu(state, &state->menu_list.scoreboard);
    }

    return 0;
}

internal void
draw_host_menu(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.4f);

    if (!menu->initialized) {
        menu->initialized = true;
    
        menu->sections = { 2, 2 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 2, 2 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    menu->gui.start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );

    //menu_text(menu, "Enter Port:", { 231, 213, 36,  1 }, { 0, 0 }, { 2, 1 }); 
    
    if (menu_textbox(menu, "Port:", state->port, *input, { 0, 0 }, { 2, 1 })) {

    }

    if (menu_button(menu, "Host", *input, { 0, 1 }, { 1, 1 })) {
        state->client_game_index = 0;
        online.server_handle = os_create_thread(play_nine_server, (void*)state);
    }
    if (menu_button(menu, "Back", *input, { 1, 1 }, { 1, 1 })) {
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
        menu->sections = { 2, 4 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 2, 4 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    menu->gui.start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    Vector4 text_color = play_nine_yellow;

    menu_textbox(menu, "Name:", state->name, *input, { 0, 0 }, { 2, 1 });
    menu_textbox(menu, "IP:",   state->ip,   *input, { 0, 1 }, { 2, 1 });
    menu_textbox(menu, "Port:", state->port, *input, { 0, 2 }, { 2, 1 });

    if (menu_button(menu, "Join", *input, { 0, 3 }, { 1, 1 })) {
        if (qsock_client(&state->client, state->ip, state->port, TCP)) {
            online.client_handle = os_create_thread(play_nine_client_recv, (void*)state);
            state->is_client = true;            
            client_set_name(state->client, state->name);
        } else {
            add_onscreen_notification(&state->notifications, "Unable to join");
        }
    }
    if (menu_button(menu, "Back", *input, { 1, 3 }, { 1, 1 })) {
        menu->hot_section = menu->interact_region[0];
        state->menu_list.mode = MAIN_MENU;
    }
}

internal void
draw_settings_menu(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    if (!menu->initialized) {
        menu->initialized = true;
        menu->sections = { 1, 4 };
        menu->interact_region[0] = { 0, 1 };
        menu->interact_region[1] = { 1, 4 };
        menu->hot_section = menu->interact_region[0];
    }

    menu_set_input(menu, input);
    menu->gui.start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    menu_text(menu, "Settings", play_nine_yellow, { 0, 0 }, { 1, 1 }); 
    if (menu_button(menu, "Video Settings", *input, { 0, 1 }, { 1, 1 })) {
        state->menu_list.mode = VIDEO_SETTINGS_MENU;
    }
    if (menu_button(menu, "Test Menu", *input, { 0, 2, }, { 1, 1 })) {
        state->game = get_test_game();
        state->menu_list.mode = SCOREBOARD_MENU;    
    }
    if (menu_button(menu, "Back", *input, { 0, 3 }, { 1, 1 })) {
        menu->hot_section = menu->interact_region[0];
        state->menu_list.mode = state->menu_list.previous_mode;
    }
}

internal void
draw_video_settings_menu(Menu *menu, State *state, Menu_Input *input, Vector2_s32 window_dim) {
    if (!menu->initialized) {
        menu->initialized = true;
        menu->sections = { 1, 4 };
        menu->interact_region[0] = { 0, 1 };
        menu->interact_region[1] = { 1, 3 };
        menu->hot_section = menu->interact_region[0];
    }
    
    menu->rect = get_centered_rect({ 0, 0 }, cv2(window_dim), 0.5f, 0.5f);
    menu_set_input(menu, input);
    menu->gui.start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    if (on_up(state->controller.pause)) {
        menu->hot_section = menu->interact_region[0];
        state->menu_list.mode = SETTINGS_MENU;
    }

    menu_text(menu, "Video Settings", play_nine_yellow, { 0, 0 }, { 1, 1 }); 
    if (menu_button(menu, "Fullscreen", *input, { 0, 1 }, { 1, 1 })) {
        
    }
    if (menu_button(menu, "Back", *input, { 0, 2 }, { 1, 1 })) {
        menu->hot_section = menu->interact_region[0];
        state->menu_list.mode = SETTINGS_MENU;
    }

}
