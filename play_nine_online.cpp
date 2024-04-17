/*
DWORD WINAPI
play_nine_client(void *parameters) {
    s32 bytes = os_send(online.sock, 0, 0, 0);

    return 0;
}
*/

internal void
close_server() {
    os_terminate_thread(online.server_handle);
    //TerminateThread(online.server_handle, NULL);
    online.close_threads = true;
    for (u32 i = 0; i < 5; i++) {
        if (online.players[i].in_use)
            WaitForSingleObject((HANDLE)online.players[i].thread_handle, INFINITE);
    }
    qsock_free_socket(online.sock);
}

internal void
server_disconnect_client(Online_Player *player) {
    Play_Nine_Packet return_packet = {};
    return_packet.type = CLOSE_CONNECTION;
    qsock_send(online.sock, &player->sock, (const char *)&return_packet, sizeof(return_packet));
    
    player->in_use = false;
    qsock_free_socket(player->sock);
    TerminateThread((HANDLE)player->thread_handle, NULL);
}

DWORD WINAPI
play_nine_server_com(void *parameters) {
    Online_Player *player = (Online_Player *)parameters;
    State *state = player->state;
    char buffer[sizeof(Play_Nine_Packet)];

    while(1) {
        s32 bytes = qsock_recv(online.sock, &player->sock, buffer, sizeof(Play_Nine_Packet));

        if (bytes <= 0) {
            logprint("play_nine_server_com()", "recv_from failed\n");
            continue;
        }

        Play_Nine_Packet *packet = (Play_Nine_Packet *)&buffer;
        switch(packet->type) {
            case SET_NAME: {
                platform_memory_copy(state->game.players[player->game_index].name, packet->buffer, MAX_NAME_SIZE);
            } break;

            case GET_GAME: {
                if (online.close_threads) {
                    server_disconnect_client(player); // closes thread
                }

                Play_Nine_Packet return_packet = {};
                return_packet.type = SET_GAME;
                return_packet.mode = state->menu_list.mode;
                return_packet.game_index = player->game_index;
                return_packet.pile_rotation = state->game_draw.camera_rotation;
                return_packet.game = state->game;
                qsock_send(online.sock, &player->sock, (const char *)&return_packet, sizeof(return_packet));
            } break;

            case SET_SELECTED: {
                os_wait_mutex(state->selected_mutex);
                if (state->game.active_player == packet->game_index)
                    platform_memory_copy(state->selected, packet->selected, sizeof(packet->selected[0]) * SELECTED_SIZE);
                os_release_mutex(state->selected_mutex);
            } break;

            case CLOSE_CONNECTION: {
                qsock_free_socket(player->sock);

                os_wait_mutex(state->mutex);

                add_onscreen_notification(&state->notifications, "Player Left");
                
                // @SPECIAL case
                if (state->menu_list.mode == SCOREBOARD_MENU) {
                    state->menu_list.mode = LOCAL_MENU;
                    state->menu_list.scoreboard.hot_section = { 0, 0 };
                }

                if (state->game.num_of_players - 1 == player->game_index) {
                    state->game.num_of_players--;
                    remove_online_player(&state->game, player->game_index);
                } else {
                    remove_player_index(&state->game, player->game_index);
                }

                os_release_mutex(state->mutex);

                player->in_use = false;

                TerminateThread((HANDLE)player->thread_handle, NULL);
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

DWORD WINAPI
play_nine_server(void *parameters) {
    State *state = (State *)parameters;
    online.close_threads = false;

    if (qsock_server(&online.sock, state->port, TCP)) {
        state->menu_list.mode = LOCAL_MENU;
        state->is_server = true;
        state->game.num_of_players = 1;
        default_player_name_string(state->game.players[0].name, 0);
    } else {
        add_onscreen_notification(&state->notifications, "Unable to start server"); 
        return 0;
    }

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
            char buffer[sizeof(Play_Nine_Packet)];
            s32 bytes = qsock_recv(online.sock, &client_sock, buffer, sizeof(Play_Nine_Packet));

            Play_Nine_Packet return_packet = {};
            return_packet.type = CLOSE_CONNECTION;
            qsock_send(online.sock, &client_sock, (const char *)&return_packet, sizeof(return_packet));
            qsock_free_socket(client_sock);

            online.players[player_index].in_use = false;

            continue;
        }
        //state->game.num_of_players++;
        add_player(&state->game, false);
        os_release_mutex(state->mutex);

        online.players[player_index].thread_handle = os_create_thread(play_nine_server_com, (void*)&online.players[player_index]);
    }

    return 0;
}

//
// Client
//

// returns true if the connection was closed
internal bool8
client_get_game(QSock_Socket sock, State *state) {
    Play_Nine_Packet packet = {};
    packet.type = GET_GAME;
    qsock_send(sock, NULL, (const char *)&packet, sizeof(packet));
    SDL_Delay(100);
    char buffer[sizeof(Play_Nine_Packet)];
    s32 bytes = qsock_recv(sock, NULL, buffer, sizeof(Play_Nine_Packet));
    if (bytes > 0) {
        os_wait_mutex(state->mutex);        
        Play_Nine_Packet *recv_packet = (Play_Nine_Packet *)&buffer;
        switch(recv_packet->type) {
            case SET_GAME: {
                state->game = recv_packet->game;
                if (state->menu_list.mode != PAUSE_MENU && recv_packet->mode != PAUSE_MENU || (recv_packet->mode != PAUSE_MENU && recv_packet->mode != IN_GAME)) {
                    state->previous_menu = state->menu_list.mode;
                    state->menu_list.mode = recv_packet->mode;
                    if (state->menu_list.mode == IN_GAME && state->previous_menu != IN_GAME)
                        load_name_plates(&state->game, &state->game_draw);
                }
                state->client_game_index = recv_packet->game_index;
                if (!state->game_draw.camera_rotation.rotating && recv_packet->pile_rotation.signal) {
                    state->game_draw.camera_rotation = {
                        false,
                        true,
                        state->game_draw.degrees_between_players,
                        0.0f,
                        -state->game_draw.rotation_speed
                    };
                }
            } break;

            case CLOSE_CONNECTION: {
                qsock_free_socket(sock);
                state->is_client = false;
                state->menu_list.mode = MAIN_MENU;
                return true;
            } break;
        }
        os_release_mutex(state->mutex);
    } else {
        logprint("client_get_game()", "Received no bytes\n");
    }

    return false;
}

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
    TerminateThread((HANDLE)online.client_handle, NULL);
}

// thread that continually calls get game
DWORD WINAPI
play_nine_client(void *parameters) {
    State *state = (State *)parameters;

    while (1) {
        //os_wait_mutex(online.mutex);

        if (client_get_game(state->client, state)) {
            add_onscreen_notification(&state->notifications, "Connection Closed");
            break;
        }

        //os_release_mutex(online.mutex);
    }

    return 0;
}
