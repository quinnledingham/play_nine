internal bool8
play_nine_recv(Play_Nine_Packet *packet, QSock_Socket self, QSock_Socket *other) {
    char *buffer = (char *)packet;
    
    s32 bytes_received = 0;
    s32 bytes_left = sizeof(Play_Nine_Packet);
    do {
        s32 bytes = qsock_recv(self, other, buffer, bytes_left);

        if (bytes <= 0) {
            logprint("play_nine_recv()", "received no bytes\n");
            return 1;
        }

        buffer += bytes;
        bytes_received += bytes;
        bytes_left -= bytes;
        
        //print("received %d bytes\n", bytes_received);
    } while(bytes_left > 0);

    return 0;
}

//
// Server
//

internal void
server_disconnect_client(Online_Player *player) {
    Play_Nine_Packet return_packet = {};
    return_packet.type = CLOSE_CONNECTION;
    qsock_send(online.sock, &player->sock, (const char *)&return_packet, sizeof(return_packet));
    
    player->in_use = false;
    qsock_free_socket(player->sock);
    os_terminate_thread(player->thread_handle);
}

internal void
close_server() {

#ifdef STEAM

    SteamMatchmaking()->LeaveLobby(steam_manager->lobby_id);

#endif // STEAM

    os_terminate_thread(online.server_handle);
    online.close_threads = true;
    for (u32 i = 0; i < 5; i++) {
        if (online.players[i].in_use) {
            server_disconnect_client(&online.players[i]);
        }
    }
    qsock_free_socket(online.sock);
}

internal void
server_send_packet_all(Play_Nine_Packet *packet) {
    for (u32 i = 0; i < 5; i++) {
        if (online.players[i].in_use) {
            packet->game_index = online.players[i].game_index;    
            qsock_send(online.sock, &online.players[i].sock, (const char *)packet, sizeof(*packet));
        }
    } 
}

internal void
server_send_game(Game *game) {
    Play_Nine_Packet return_packet = {};
    return_packet.type = SET_GAME;
    return_packet.game = *game;
    server_send_packet_all(&return_packet);
}

internal void
server_send_game(Game *game, Draw_Signal *signals, u32 signals_count) {
    Play_Nine_Packet packet = {};
    packet.type = SET_GAME;
    packet.game = *game;
    packet.signals_count = signals_count;

    for (u32 i = 0; i < signals_count; i++) {
        if (!signals[i].in_use)
            continue;
        packet.signals[i] = signals[i];
    }
    
    server_send_packet_all(&packet);
}


internal void
server_send_menu_mode(enum Menu_Mode mode) {
    Play_Nine_Packet packet = {};
    packet.type = SET_MENU_MODE;
    packet.mode = mode;
    server_send_packet_all(&packet);
};

THREAD_RETURN play_nine_server_com(void *parameters) {
    Online_Player *player = (Online_Player *)parameters;
    State *state = player->state;

    while(1) {
        Play_Nine_Packet packet = {};
        if (play_nine_recv(&packet, online.sock, &player->sock)) {
            logprint("play_nine_server_com()", "recv_from failed\n");
            continue;
        }

        switch(packet.type) {
            case SET_NAME: {
                platform_memory_copy(state->game.players[player->game_index].name, packet.buffer, MAX_NAME_SIZE);
                server_send_game(&state->game);        
            } break;

            case SET_SELECTED: {
                os_wait_mutex(state->selected_mutex);
                if (state->game.active_player == packet.game_index)
                    platform_memory_copy(state->selected, packet.selected, sizeof(packet.selected[0]) * SELECTED_SIZE);
                os_release_mutex(state->selected_mutex);
            } break;

            case CLOSE_CONNECTION: {
                qsock_free_socket(player->sock);

                os_wait_mutex(state->mutex);

                add_onscreen_notification(&state->notifications, "Player Left");
                
                // @SPECIAL case
                if (state->menu_list.mode == SCOREBOARD_MENU) {
                    state->menu_list.mode = LOCAL_MENU;
                    state->menu_list.menus[SCOREBOARD_MENU].hover_section = { 0, 0 };
                }
                
                remove_player_index(&state->game, player->game_index);
                remove_online_player(&state->game, player->game_index);

                if (state->game.active_player == player->game_index) {
                    decrement_player(&state->game.active_player, state->game.num_of_players);
                    next_player(&state->game);
                }
                
                add_draw_signal(draw_signals, SIGNAL_ALL_PLAYER_CARDS);
                add_draw_signal(draw_signals, SIGNAL_UNLOAD_NAME_PLATE, 0, player->game_index);

                os_release_mutex(state->mutex);

                player->in_use = false;

                os_terminate_thread(player->thread_handle);
            }            
        }
    };

    return 0;
}

internal u32
find_free_player_index() {
    for (u32 i = 0; i < 5; i++) {
        if (!online.players[i].in_use) {
            online.players[i].in_use = true;
            return i;
        }
    }
    logprint("find_free_player_index()", "Didn't find free index\n");
    return 0;
}

internal void
goto_local_menu(State *state, Menu_Mode previous_mode, Online_Mode online_mode) {
    state->menu_list.previous_mode = previous_mode;
    state->menu_list.mode = LOCAL_MENU;
    state->mode = online_mode;
    state->game.num_of_players = 1;
    u32_to_char_array(state->num_of_holes, TEXTBOX_SIZE, state->game.holes_length);
    default_player_name_string(state->game.players[0].name, 1);
}

THREAD_RETURN play_nine_server(void *parameters) {
    State *state = (State *)parameters;
    online.close_threads = false;

    if (qsock_server(&online.sock, state->host_port, TCP)) {
        goto_local_menu(state, HOST_MENU, MODE_SERVER);
    } else {
        add_onscreen_notification(&state->notifications, "Unable to start server"); 
        return 0;
    }

#ifdef STEAM

    steam_manager->create_lobby(k_ELobbyTypeFriendsOnly, 6);

#endif // STEAM

    while (1) {
        qsock_listen(online.sock);
        QSock_Socket client_sock;
        qsock_accept(&online.sock, &client_sock);

        u32 player_index = find_free_player_index();
        online.players[player_index].sock = client_sock;
        online.players[player_index].game_index = state->game.num_of_players;
        online.players[player_index].state = state;

        os_wait_mutex(state->mutex);
        if (state->menu_list.mode != LOCAL_MENU) {
            os_release_mutex(state->mutex);

            Play_Nine_Packet recv_packet = {};
            play_nine_recv(&recv_packet, online.sock, &client_sock);

            Play_Nine_Packet return_packet = {};
            return_packet.type = CLOSE_CONNECTION;
            qsock_send(online.sock, &client_sock, (const char *)&return_packet, sizeof(return_packet));
            qsock_free_socket(client_sock);

            online.players[player_index].in_use = false;

            continue;
        }
        add_player(&state->game, false);
        
        Play_Nine_Packet packet = {};
        packet.type = SET_MENU_MODE;
        packet.mode = state->menu_list.mode;
        qsock_send(online.sock, &client_sock, (const char *)&packet, sizeof(packet));

        os_release_mutex(state->mutex);

        online.players[player_index].thread_handle = os_create_thread(play_nine_server_com, (void*)&online.players[player_index]);

    }

    return 0;
}

//
// Client
//

internal void
client_set_name(QSock_Socket sock, const char *name) {
    Play_Nine_Packet packet = {};
    packet.type = SET_NAME;
    platform_memory_copy(packet.buffer, (void*)name, get_length(name));
    qsock_send(sock, NULL, (const char *)&packet, sizeof(packet));
}

internal void
client_set_selected(QSock_Socket sock, bool8 selected[SELECTED_SIZE], u32 index) {
    Play_Nine_Packet packet = {};
    packet.type = SET_SELECTED;
    packet.game_index = index;
    platform_memory_copy(packet.selected, selected, sizeof(selected[0]) * SELECTED_SIZE);
    qsock_send(sock, NULL, (const char *)&packet, sizeof(packet));
}

internal void
client_close_connection(QSock_Socket sock) {
    Play_Nine_Packet packet = {};
    packet.type = CLOSE_CONNECTION;
    qsock_send(sock, NULL, (const char *)&packet, sizeof(packet));
    qsock_free_socket(sock);
    os_terminate_thread(online.client_handle);
}

THREAD_RETURN play_nine_client_recv(void *parameters) {
    State *state = (State *)parameters;

    while(1) {
        Play_Nine_Packet recv_packet = {};
        
        if (play_nine_recv(&recv_packet, online.sock, NULL)) {
            return 0;
        }
        
        os_wait_mutex(state->mutex);
        switch(recv_packet.type) {
            case SET_GAME: {
                state->game = recv_packet.game;
                state->client_game_index = recv_packet.game_index;
                for (u32 i = 0; i < recv_packet.signals_count; i++) {
                    if (recv_packet.signals[i].in_use)
                        add_draw_signal(draw_signals, recv_packet.signals[i]);
                }
            } break;

            case SET_MENU_MODE: {
                state->menu_list.mode = recv_packet.mode;
            } break;

            case ADD_DRAW_SIGNAL: {
                add_draw_signal(draw_signals, recv_packet.signal);
            } break;

            case ADD_ALL_DRAW_SIGNALS: {
                for (u32 i = 0; i < recv_packet.signals_count; i++) {
                    if (!recv_packet.signals[i].sent)
                        add_draw_signal(draw_signals, recv_packet.signals[i]);
                }
            } break;

            // if the server tells the client to disconnect
            case CLOSE_CONNECTION: {
                qsock_free_socket(online.sock);
                state->mode = MODE_LOCAL;
                state->menu_list.mode = JOIN_MENU;
                add_onscreen_notification(&state->notifications, "Connection Closed");
                os_release_mutex(state->mutex);
                return 0;
            } break;
        }
        os_release_mutex(state->mutex);
    }

    return 0;
}

THREAD_RETURN play_nine_client_join(void *parameters) {
    State *state = (State *)parameters;
    
    os_wait_mutex(state->mutex);
    state->loading_icon.enable(find_bitmap(&state->assets, "LOADING"));
    os_release_mutex(state->mutex);
    
    if (qsock_client(&online.sock, state->join_ip, state->join_port, TCP)) {
        os_wait_mutex(state->mutex);
        online.client_handle = os_create_thread(play_nine_client_recv, (void*)state);
        state->mode = MODE_CLIENT;            
        client_set_name(online.sock, state->join_name);
    } else {
        os_wait_mutex(state->mutex);
        add_onscreen_notification(&state->notifications, "Unable to join");
    }

    state->loading_icon.disable();
    os_release_mutex(state->mutex); // wait in either of the if branches
    
    return 0;
}
