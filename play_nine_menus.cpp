internal void
menu_update_active(s32 *active, s32 lower, s32 upper, Button increase, Button decrease) {
    if (on_down(increase)) {
        (*active)++;
        if (*active > upper)
            *active = upper;
    }
    if (on_down(decrease)) {
        (*active)--;
        if (*active < lower)
            *active = lower;
    }
}

// returns game mode
internal s32
draw_main_menu(State *state, Menu *main_menu, Font *font, Controller *controller, Vector2_s32 window_dim) {
    menu_update_active(&state->menu_list.main.active, 0, 2, state->controller.backward,  state->controller.forward);

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    main_menu->rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    if (!main_menu->initialized) {
        main_menu->initialized = true;
        main_menu->font = font;

        main_menu->button_style.default_back_color = { 231, 213, 36,  1 };
        main_menu->button_style.active_back_color  = { 240, 229, 118, 1 };
        main_menu->button_style.default_text_color = { 39,  77,  20,  1 };
        main_menu->button_style.active_text_color  = { 39,  77,  20,  1 };;
        
        u32 buttons_count = 3;    
    }

    main_menu->button_style.dim = { main_menu->rect.dim.x, main_menu->rect.dim.y / float32(3) };

    bool8 select = on_down(controller->select);
    u32 index = 0;

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );
    draw_rect(main_menu->rect.coords, 0, main_menu->rect.dim, { 0, 0, 0, 0.2f} );

    if (menu_button(main_menu, "Local",  index++, main_menu->active, select)) {
        state->menu_list.mode = LOCAL_MENU;
    }
    if (menu_button(main_menu, "Online", index++, main_menu->active, select))
        state->menu_list.mode = IN_GAME;    
    if (menu_button(main_menu, "Quit",   index++, main_menu->active, select)) 
        return true;

    return false;
}

internal void
default_player_name_string(char buffer[MAX_NAME_SIZE], u32 number) {
    platform_memory_set((void*)buffer, 0, MAX_NAME_SIZE);
    buffer[8] = 0;
    memcpy(buffer, "Player ", 7);
    buffer[7] = number + 48;
}

internal s32
draw_local_menu(State *state, Menu *menu, Font *font, Controller *controller, Vector2_s32 mouse, enum Input_Type active_input_type, Vector2_s32 window_dim) {
    Game *game = &state->game;

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);
    menu->rect = get_centered_rect(window_rect, 0.9f, 0.7f);

    if (!menu->initialized) {
        menu->initialized = true;
        menu->font = font;

        menu->button_style.default_back_color = { 231, 213, 36,  1 };
        menu->button_style.active_back_color  = { 240, 229, 118, 1 };
        menu->button_style.default_text_color = { 39,  77,  20,  1 };
        menu->button_style.active_text_color  = { 39,  77,  20,  1 };;

        menu->sections = { 2, 4 };

        game->num_of_players = 1;
        default_player_name_string(game->players[0].name, 0);
    }

    bool8 select = on_down(controller->select) || on_down(controller->mouse_left);
    u32 index = 0;

    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 39,77,20, 1 } );
    draw_rect(menu->rect.coords, 0, menu->rect.dim, { 0, 0, 0, 0.2f} );

    menu->sections.y = 3 + game->num_of_players;

    Menu_Input input = {
        select,
        active_input_type,

        controller->forward,
        controller->backward,
        controller->left,
        controller->right,

        menu->active_section,
        &menu->active_section,

        mouse
    };

    s32 menu_row = 0;
    for (menu_row; menu_row < (s32)game->num_of_players; menu_row++) {
        menu_button(menu, game->players[menu_row].name, input, { 0, menu_row }, { 2, 1 });
    }

    if (menu_button(menu, "+ Player", input, { 0, menu_row }, { 1, 1 })) {
        if (game->num_of_players != 6) {
            default_player_name_string(game->players[game->num_of_players].name, game->num_of_players);
            game->num_of_players++;
            menu->active_section.y++;
        }
    }
    if (menu_button(menu, "- Player", input, { 1, menu_row }, { 1, 1 })) {
        if (game->num_of_players != 1) {
            game->num_of_players--;
            menu->active_section.y--;
        }
    }

    if (menu_button(menu, "+ Bot", input, { 0, menu_row + 1 }, { 1, 1 })) {
        
    }
    if (menu_button(menu, "- Bot", input, { 1, menu_row + 1 }, { 1, 1 })) {
        
    }

    if (menu_button(menu, "Start", input, { 0, menu_row + 2 }, { 1, 1 })) {
        if (game->num_of_players != 1) {
            state->menu_list.mode = IN_GAME;
            start_game(&state->game, game->num_of_players);
            state->game_draw.degrees_between_players = 360.0f / float32(state->game.num_of_players);
            state->game_draw.radius = 8.0f;
        }
    }
    if (menu_button(menu, "Back", input, { 1, menu_row + 2 }, { 1, 1 })) {
        state->menu_list.mode = MAIN_MENU;
        game->num_of_players = 1;
        menu->active_section = { 0, 0 };
    }

    return false;
}