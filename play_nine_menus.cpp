internal void
quit_to_main_menu(State *state, Menu *menu) {
    os_wait_mutex(state->mutex);

    switch(state->mode) {
        case MODE_CLIENT: {
            client_close_connection(online.sock);
        } break;

        case MODE_SERVER: {
            close_server();
        } break;
    }

    state->menu_list.update_close(MAIN_MENU);
    state->game.num_of_players = 1;
    state->mode = MODE_LOCAL;

    os_release_mutex(state->mutex);
}

// returns game mode
internal s32
draw_main_menu(State *state, Menu *menu, Vector2_s32 window_dim) {
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->gui.rect = get_centered_rect(window_rect, 0.6f, 0.6f);

    menu->sections = { 1, 7 };
    menu->interact_region[0] = { 0, 2 };
    menu->interact_region[1] = menu->sections;
    
    menu->start();

    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );    
    menu_text(menu, "play_nine", play_nine_yellow, { 0, 0 }, { 1, 2 }); 

    if (menu_button(menu, "Local", { 0, 2 }, { 1, 1 })) {
        goto_local_menu(state, MAIN_MENU, MODE_LOCAL);
    }
    if (menu_button(menu, "Host Online", { 0, 3 }, { 1, 1 })) {
        state->menu_list.update(HOST_MENU);
    }
    if (menu_button(menu, "Join Online", { 0, 4 }, { 1, 1 })) {
        state->menu_list.update(JOIN_MENU);
    }
    if (menu_button(menu, "Settings", { 0, 5 }, { 1, 1 })) {
        state->menu_list.previous_mode = MAIN_MENU;
        state->menu_list.update(SETTINGS_MENU);
    }
    if (menu_button(menu, "Quit", { 0, 6 }, { 1, 1 })) 
        return true;

#ifdef DEBUG
    draw_card_bitmaps(card_bitmaps, window_dim);
#endif // DEBUG

    menu->end();

    return false;
}

internal void
draw_bot_icon(Menu *menu, Vector2_s32 section_coords, Vector2_s32 draw_dim) {
    Vector2 coords = get_screen_coords(menu, section_coords);
    Vector2 dim = get_screen_dim(menu, draw_dim);
    Vector2 bot_dim = dim * 0.6f;
    bot_dim.x = bot_dim.y;
    coords.x += dim.x;
    coords.x -= (bot_dim.y / 2.0f);
    coords.x -= bot_dim.x / 2.0f;

    coords.y += (dim.y / 2.0f) - (bot_dim.y / 2.0f);
    coords.y += bot_dim.y / 2.0f;

    Shader *shader = find_shader(global_assets, "TEXT");
    render_bind_pipeline(&shader->pipeline);

    Descriptor v_color_set = render_get_descriptor_set(&layouts[4]);
    render_bind_descriptor_sets(v_color_set, (void *)&play_nine_green);

    Object object = {};

    Descriptor desc = render_get_descriptor_set(&layouts[2]);
    object.index = render_set_bitmap(&desc, find_bitmap(global_assets, "BOT"));
    render_bind_descriptor_set(desc);

    Vector3 coords_v3 = { coords.x, coords.y, 0 };
    Quaternion rotation_quat = get_rotation(0, { 0, 0, 1 });
    Vector3 dim_v3 = { bot_dim.width, bot_dim.height, 1 };
    object.model = create_transform_m4x4(coords_v3, rotation_quat, dim_v3);

    render_push_constants(SHADER_STAGE_VERTEX, &object, sizeof(Object));

    render_draw_mesh(&shapes.rect_mesh);
}

internal s32
draw_local_menu(State *state, Menu *menu, bool8 full_menu, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->gui.rect = get_centered_rect(window_rect, 0.8f, 0.8f);

    menu->sections = { 4, 7 };
    menu->interact_region[0] = { 0, 1 };
    menu->interact_region[1] = { 4, 7 };

    menu->start();

    if (on_up(state->controller.pause)) {
        state->menu_list.update_close(state->menu_list.previous_mode);
    }
    
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );
    //draw_rect(menu->gui.rect.coords, 0, menu->gui.rect.dim, { 255, 0, 0, 1 });

    float32 x_section = menu->gui.rect.dim.x / menu->sections.x;
    float32 y_section = menu->gui.rect.dim.y / menu->sections.y;
    Vector2 dark_rect_coords = { menu->gui.rect.coords.x, menu->gui.rect.coords.y + (1 * y_section) };
    Vector2 dark_rect_dim = { menu->gui.rect.dim.x - (2 * x_section), menu->gui.rect.dim.y - (1 * y_section) };
    draw_rect(dark_rect_coords, 0, dark_rect_dim, { 0, 0, 0, 0.2f} );

    const char *lobby_name;
    const char *back_label;
    if (state->mode == Online_Mode::MODE_LOCAL) {
        lobby_name = "Local Lobby";
        back_label = "Main Menu";
    } else if (state->mode == Online_Mode::MODE_CLIENT) {
        lobby_name = "Online Lobby";
        back_label = "Leave Game";
    } else if (state->mode == Online_Mode::MODE_SERVER) {
        lobby_name = "Online Lobby";
        back_label = "Close Game";
    }
    
    menu_text(menu, lobby_name,  play_nine_yellow, { 0, 0 }, { 4, 1 });
        
    s32 menu_row = 1;
    for (u32 player_index = 0; player_index < (s32)game->num_of_players; player_index++) {
        Player *player = &game->players[player_index];
        
        Vector2_s32 section_coords = { 0, menu_row };
        Vector2_s32 section_dim = { 2, 1 };
        Vector2_s32 draw_dim = section_dim;
        
        if (player_index == game->num_of_players - 1) {
            section_dim.y = 2 + 6 - (s32)game->num_of_players;
        }

        if (state->mode == MODE_LOCAL || player_index == state->client_game_index) {
            if (menu_textbox(menu, "Name:", player->name, section_coords, section_dim, draw_dim)) {
                if (state->mode == MODE_CLIENT)
                    client_set_name(online.sock, player->name);
                else if (state->mode == MODE_SERVER)
                    server_send_game(&state->game);            
            }
        } else {
            menu_button(menu, player->name, section_coords, section_dim, draw_dim);
        }

        if (player->is_bot) {
            draw_bot_icon(menu, section_coords, draw_dim);
        }

        menu_row++;
    }

    menu_row = 1;

    if (full_menu) {
        if (state->mode != MODE_SERVER) {
            if (menu_button(menu, "+ Player", { 2, menu_row }, { 1, 1 })) {
                add_player(game, false);
            }
            if (menu_button(menu, "- Player", { 3, menu_row++ }, { 1, 1 })) {
                remove_player(game, false);
            }
        } 

        if (menu_button(menu, "+ Bot", { 2, menu_row }, { 1, 1 })) {
            add_player(game, true);
            server_send_game(&state->game);
        }
        if (menu_button(menu, "- Bot", { 3, menu_row++ }, { 1, 1 })) {
            s32 removed_index = remove_player(game, true);
            if (removed_index != -1 && state->mode == MODE_SERVER)
                remove_online_player(&state->game, removed_index);
            server_send_game(&state->game);
        }
        if (menu_textbox(menu, "# of Holes:", state->num_of_holes, { 2, menu_row++ }, { 2, 1 })) {
            s32 last_holes_length = state->game.holes_length;
            char_array_to_s32(state->num_of_holes, &state->game.holes_length);
            if (state->game.holes_length < 1) {
                state->game.holes_length = last_holes_length;
            }
            s32_to_char_array(state->num_of_holes, TEXTBOX_SIZE, state->game.holes_length);
        }

        if (menu_button(menu, "Start Game", { 2, menu_row++ }, { 2, 1 })) {
            if (game->num_of_players != 1) {
                state->menu_list.update_close(IN_GAME);
                start_game(&state->game, game->num_of_players);
                
                add_draw_signal(draw_signals, SIGNAL_ALL_PLAYER_CARDS);
                add_draw_signal(draw_signals, SIGNAL_NAME_PLATES);
                
                if (state->mode == MODE_SERVER) {
                    server_send_game(game, draw_signals, DRAW_SIGNALS_AMOUNT);
                    server_send_menu_mode(state->menu_list.mode);
                }
#if STEAM
                u32 port;
                char_array_to_u32(state->host_port, &port);
                SteamMatchmaking()->SetLobbyGameServer(steam_manager->lobby_id, 0x7f000001, (u16)port, k_steamIDNil);
#endif // STEAM
            } else {
                add_onscreen_notification(&state->notifications, "Not enough players");
            }
        }
    } 

    Vector2_s32 back_section_dim = { 2, 2 };
    if (state->mode == MODE_SERVER) {
        back_section_dim = { 2, 5 };
    } else if (state->mode == MODE_CLIENT) {
        back_section_dim = { 2, 6 };
    }
    
    if (menu_button_confirm(menu, back_label, "(Again to confirm)", { 2, menu_row++ }, back_section_dim, { 2, 1 })) {
        quit_to_main_menu(state, menu);
    }

    menu->end();

    return false;
}

internal s32
draw_pause_menu(State *state, Menu *menu, bool8 full_menu, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect   = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->gui.rect = get_centered_rect(window_rect, 0.5f, 0.3f);

    if (full_menu) {
        menu->sections = { 1, 3 };
        menu->interact_region[0] = { 0, 0 };
        menu->interact_region[1] = { 1, 3 };
    } else {
        menu->sections = { 1, 2 };
        menu->interact_region[0] = { 0, 1 };
        menu->interact_region[1] = { 1, 2 };
    }

    menu->start();
    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 0, 0, 0, 0.5f} );

    s32 menu_row = 0;

    if (full_menu) {
        if (menu_button_confirm(menu, "End Game", "(Again to confirm)", { 0, menu_row++ }, { 1, 1 })) {
            state->menu_list.update_close(LOCAL_MENU);

            if (state->mode == MODE_SERVER)
                server_send_menu_mode(state->menu_list.mode);
        }
    }
    
    if (menu_button(menu, "Settings", { 0, menu_row++ }, { 1, 1 })) {
        state->menu_list.previous_mode = PAUSE_MENU;
        state->menu_list.update(SETTINGS_MENU);
    }

    if (menu_button_confirm(menu, "Main Menu", "(Again to confirm)", { 0, menu_row++ }, { 1, 1 })) {
        quit_to_main_menu(state, menu);
    }
    menu->end();

    return false;
}

/*
0 do nothing
1 next hole
2 quit game
*/

internal s32
draw_scoreboard(Menu *menu, State *state, bool8 full_menu, Vector2_s32 window_dim) {
    Game *game = &state->game;
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->gui.rect = get_centered_rect(window_rect, 0.9f, 0.9f);

    Vector2_s32 scroll = {};
    s32 scroll_length = 8;
    menu->sections = { (s32)game->num_of_players + 1, scroll_length + 3 };
    menu->interact_region[0] = { 0, scroll_length + 2 };
    menu->interact_region[1] = { (s32)game->num_of_players + 1, scroll_length + 2 };

    if ((s32)game->holes_played <= scroll_length)
        scroll = { 1, (s32)game->holes_played };
    else
        scroll = { (s32)game->holes_played - scroll_length + 1, (s32)game->holes_played + 1 };

    menu->start();
    
    if (on_up(state->controller.pause)) {
        state->menu_list.update_close(IN_GAME);
    }
    
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );
    draw_rect(menu->gui.rect, { 0, 0, 0, 0.2f} );

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
        if (i + 1 < scroll.x || i + 1 > scroll.y) {
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
        //u32 player_total = 0;
        //for (u32 i = 0; i < game->holes_played; i++)
        //    player_total += game->players[player_index].scores[i];
        platform_memory_set(total_text, 0, 4);
        s32_to_char_array(total_text, 4, game->players[player_index].total_score);
        menu_text(menu, total_text, text_color, { player_index + 1, scroll_length + 1 }, { 1, 1 }); 
    }

    if (menu_button(menu, "Back", { 0, scroll_length + 2 }, { (s32)game->num_of_players + 1, 1 })) {
        state->menu_list.update_close(IN_GAME);
    }
    
    menu->end();

    return 0;
}

internal void
draw_host_menu(Menu *menu, State *state, Vector2_s32 window_dim) {
    menu->gui.rect = get_centered_rect({ 0, 0 }, cv2(window_dim), 0.5f, 0.4f);

    menu->sections = { 2, 2 };
    menu->interact_region[0] = { 0, 0 };
    menu->interact_region[1] = { 2, 2 };
    
    menu->start();
        
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green);
    
    if (menu_textbox(menu, "Port:", state->host_port, { 0, 0 }, { 2, 1 })) {

    }

    if (menu_button(menu, "Host", { 0, 1 }, { 1, 1 })) {
        state->client_game_index = 0;
        online.server_handle = os_create_thread(play_nine_server, (void*)state);
    }
    if (menu_button(menu, "Back", { 1, 1 }, { 1, 1 }) || on_up(state->controller.pause)) {
        state->menu_list.update_close(MAIN_MENU);
    }
              
    menu->end();
    
    draw_input_prompt({ 200, 200, 0 }, state->controller.pause);
}

internal void
draw_join_menu(Menu *menu, State *state, Vector2_s32 window_dim) {
    menu->gui.rect = get_centered_rect({ 0, 0 }, cv2(window_dim), 0.5f, 0.5f);

    menu->sections = { 2, 4 };
    menu->interact_region[0] = { 0, 0 };
    menu->interact_region[1] = { 2, 4 };
    
    menu->start();
        
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    menu_textbox(menu, "Name:", state->join_name, { 0, 0 }, { 2, 1 });
    menu_textbox(menu, "IP:",   state->join_ip,   { 0, 1 }, { 2, 1 });
    menu_textbox(menu, "Port:", state->join_port, { 0, 2 }, { 2, 1 });

    local_persist THREAD join_thread = 0;
    if (menu_button(menu, "Join", { 0, 3 }, { 1, 1 })) {
        join_thread = os_create_thread(play_nine_client_join, (void*)state);
    }
    if (menu_button(menu, "Back", { 1, 3 }, { 1, 1 }) || on_up(state->controller.pause)) {
        state->menu_list.update_close(MAIN_MENU);

        if (join_thread) {
            os_terminate_thread(join_thread);
            state->loading_icon.disable();
        }
    }
    menu->end();

    state->loading_icon.draw(window_dim);
}

internal void
draw_settings_menu(Menu *menu, State *state, Vector2_s32 window_dim) {
    menu->gui.rect = get_centered_rect({ 0, 0 }, cv2(window_dim), 0.5f, 0.5f);

#if DEBUG
    menu->sections = { 1, 5 };
#else
    menu->sections = { 1, 4 };
#endif // DEBUG

    menu->interact_region[0] = { 0, 1 };
    menu->interact_region[1] = menu->sections;
    
    menu->start();
    
    draw_rect({ 0, 0 }, 0, cv2(window_dim), play_nine_green );

    s32 y_index = 0;
    menu_text(menu, "Settings", play_nine_yellow, { 0, y_index++ }, { 1, 1 }); 
    if (menu_button(menu, "Video", { 0, y_index++ }, { 1, 1 })) {
        state->menu_list.mode = VIDEO_SETTINGS_MENU;
    }
    if (menu_button(menu, "Audio", { 0, y_index++ }, { 1, 1 })) {
        state->menu_list.mode = AUDIO_SETTINGS_MENU;
    }
#if DEBUG
    if (menu_button(menu, "Test Menu", { 0, y_index++ }, { 1, 1 })) {
        state->game = get_test_game();
        state->menu_list.mode = SCOREBOARD_MENU;    
    }
#endif // DEBUG
    if (menu_button(menu, "Back", { 0, y_index++ }, { 1, 1 }) || on_up(state->controller.pause)) {
        state->menu_list.update_close(state->menu_list.previous_mode);
    }
    menu->end();
}

internal void
menu_fill_resolution(char *buffer, float32 scale) {
    Vector2_s32 resolution = get_resolution(render_context.window_dim, scale);
    u32 digits_x = get_digits(resolution.x);
    u32 digits_y = get_digits(resolution.y);
    s32_to_char_array(buffer +  6 + (4 - digits_x), 5, resolution.x);
    s32_to_char_array(buffer + 13 + (4 - digits_y), 5, resolution.y);
    buffer[10] = ' ';
    buffer[17] = ')';
}

internal void
draw_video_settings_menu(Menu *menu, State *state, App_Window *window) {
    menu->sections = { 1, 6 };
    menu->interact_region[0] = { 0, 1 };
    menu->interact_region[1] = { 1, 6 };
    
    menu->gui.rect = get_centered_rect({ 0, 0 }, cv2(window->dim), 0.5f, 0.5f);
    menu->start();
    draw_rect({ 0, 0 }, 0, cv2(window->dim), play_nine_green);

    if (on_up(state->controller.pause)) {
        if (menu->gui.active == 0) {
            state->menu_list.update_close(SETTINGS_MENU);
        } else {
            // break out of dropdown menu
            menu->gui.pressed = 0;
            menu->gui.active = 0;
        }
    }

    menu_text(menu, "Video Settings", play_nine_yellow, { 0, 0 }, { 1, 1 }); 
    
    if (menu_checkbox(menu, "VSync", &render_context.vsync, { 0, 2 }, { 1, 1 })) {
        window->resized = true;
    }
    if (menu_checkbox(menu, "Anti-aliasing", &render_context.anti_aliasing, { 0, 3 }, { 1, 1 })) {
        window->resized = true;
    }
    if (menu_button(menu, "Back", { 0, 5 }, { 1, 1 })) {
        state->menu_list.update_close(SETTINGS_MENU);
    }
    
    const char *resolution_modes[4] = {
        "25%  (     x     )",
        "50%  (     x     )",
        "75%  (     x     )",
        "100% (     x     )"
    };    

    u32 char_array_size = 19; // length of one resolution_mode string
    char **resolution_chars = (char**)platform_malloc(sizeof(char*) * 4);
    for (u32 i = 0; i < ARRAY_COUNT(resolution_modes); i++) {
        resolution_chars[i] = (char *)platform_malloc(char_array_size);
        platform_memory_set(resolution_chars[i], 0, char_array_size);
        platform_memory_copy((void *)resolution_chars[i], (void *)resolution_modes[i], char_array_size);
        menu_fill_resolution(resolution_chars[i], get_resolution_scale(i));
    }
    
    if (menu_dropdown(menu, (const char **)resolution_chars, 4, &render_context.resolution_mode, { 0, 4 }, { 1, 1 })) {
        window->resized = true;
        switch(render_context.resolution_mode) {
            case RESOLUTION_480P:  render_context.resolution_scale = 0.25f; break;
            case RESOLUTION_720P:  render_context.resolution_scale = 0.5f;  break;
            case RESOLUTION_1080P: render_context.resolution_scale = 0.75f; break;
            case RESOLUTION_2160P: render_context.resolution_scale = 1.0f;  break;
        }
        render_context.update_resolution();
    }
    
    const char *fullscreen_modes[3] = {
        "Windowed",
        "Fullscreen",
        "Windowed Fullscreen"
    };
    if (menu_dropdown(menu, fullscreen_modes, 3, (u32*)&window->display_mode, { 0, 1 }, { 1, 1 })) {
        window->new_display_mode = true;
        window->resized = true;
    }
    menu->end();

    platform_free(resolution_chars[0]);
    platform_free(resolution_chars[1]);
    platform_free(resolution_chars[2]);
    platform_free(resolution_chars[3]);
    platform_free(resolution_chars);
    
}

internal void
draw_audio_settings_menu(Menu *menu, State *state, Audio_Player *player, App_Window *window) {
    menu->sections = { 1, 6 };
    menu->interact_region[0] = { 0, 1 };
    menu->interact_region[1] = { 1, 6 };
    
    menu->gui.rect = get_centered_rect({ 0, 0 }, cv2(window->dim), 0.5f, 0.5f);
    menu->start();
    draw_rect({ 0, 0 }, 0, cv2(window->dim), play_nine_green);

    if (on_up(state->controller.pause)) {
        if (menu->gui.active == 0) {
            state->menu_list.update_close(SETTINGS_MENU);
        } 
    }
    
    menu_text(menu, "Audio", play_nine_yellow, { 0, 0 }, { 1, 1 }); 

    menu_slider(menu, &player->music_volume, 6, "Music", { 0, 1 }, { 1, 2 });
    if (menu_slider(menu, &player->sound_effects_volume, 6, "Sound Effects", { 0, 3 }, { 1, 2 })) {
        play_sound("TAP");
    }

    if (menu_button(menu, "Back", { 0, 5 }, { 1, 1 })) {
        state->menu_list.update_close(SETTINGS_MENU);
    }
    
    menu->end();

}
