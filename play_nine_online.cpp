#include "play_nine_online.h"

internal s64
win32_create_mutex() {
    s64 mutex = (s64)CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL) {
        print("CreateMutex error: %d\n", GetLastError());
    }
    return mutex;
}

internal void
win32_wait_mutex(s64 handle) {
    DWORD wait_result = WaitForSingleObject((HANDLE)handle, INFINITE);
    if (wait_result == WAIT_ABANDONED) {}

    if (wait_result == WAIT_OBJECT_0) {}
}

internal void
win32_release_mutex(s64 handle) {
    if (!ReleaseMutex((HANDLE)handle)) {
        print("ReleaseMutex error: %d\n", GetLastError());
    }
}
/*
DWORD WINAPI
play_nine_client(void *parameters) {
    s32 bytes = win32_send(online.sock, 0, 0, 0);

    return 0;
}
*/

internal void
close_server() {
    TerminateThread(online.server_handle, NULL);
    online.close_threads = true;
    for (u32 i = 0; i < 5; i++) {
        if (online.players[i].in_use)
            WaitForSingleObject((HANDLE)online.players[i].thread_handle, INFINITE);
    }
    closesocket(online.sock.handle);
}

DWORD WINAPI
play_nine_server_com(void *parameters) {
    Online_Player *player = (Online_Player *)parameters;
    State *state = player->state;
    char buffer[sizeof(Play_Nine_Packet)];

    //u32 player_index = online.player_indices[online.clients_index];
    //u32 client_index = online.clients_index;
    //online.clients_index++;

    while(1) {
        s32 bytes = win32_recv_from(player->sock, buffer, sizeof(Play_Nine_Packet), 0, &player->sock.recv_info);

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
                    Play_Nine_Packet return_packet = {};
                    return_packet.type = CLOSE_CONNECTION;
                    win32_send(player->sock, (const char *)&return_packet, sizeof(return_packet), 0);
                    
                    player->in_use = false;
                    closesocket(player->sock.handle);
                    TerminateThread((HANDLE)player->thread_handle, NULL);
                }

                Play_Nine_Packet return_packet = {};
                return_packet.type = SET_GAME;
                return_packet.mode = state->menu_list.mode;
                return_packet.game_index = player->game_index;
                platform_memory_copy(return_packet.buffer, &state->game, sizeof(Game));
                win32_send(player->sock, (const char *)&return_packet, sizeof(return_packet), 0);
            } break;

            case SET_SELECTED: {
                win32_wait_mutex(state->selected_mutex);
                if (state->game.active_player == packet->game_index)
                    platform_memory_copy(state->selected, packet->selected, sizeof(packet->selected[0]) * SELECTED_SIZE);
                win32_release_mutex(state->selected_mutex);
            } break;

            case CLOSE_CONNECTION: {
                closesocket(player->sock.handle);

                win32_wait_mutex(state->mutex);

                if (state->game.num_of_players - 1 == player->game_index) {
                    state->game.num_of_players--;
                } else {
                    remove_player(&state->game, player->game_index);
                    for (u32 i = 0; i < 5; i++) {
                        if (online.players[i].in_use && online.players[i].game_index > player->game_index)
                            online.players[i].game_index--;
                    }
                }

                win32_release_mutex(state->mutex);

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

        win32_wait_mutex(state->mutex);
        state->game.num_of_players++;
        //default_player_name_string(state->game.players[online.players[player_index].game_index].name, online.players[player_index].game_index);
        win32_release_mutex(state->mutex);

        DWORD thread_id;
        online.players[player_index].thread_handle = (s64)CreateThread(0, 0, play_nine_server_com, (void*)&online.players[player_index], 0, &thread_id);
    }

    return 0;
}

internal void
client_get_game(QSock_Socket sock, State *state) {
    Play_Nine_Packet packet = {};
    packet.type = GET_GAME;
    win32_send(sock, (const char *)&packet, sizeof(packet), 0);

    char buffer[sizeof(Play_Nine_Packet)];
    s32 bytes = win32_recv(sock, buffer, sizeof(Play_Nine_Packet), 0);
    if (bytes > 0) {
        Play_Nine_Packet *recv_packet = (Play_Nine_Packet *)&buffer;
        switch(recv_packet->type) {
            case SET_GAME: {
                state->game = *(Game *)recv_packet->buffer;
                if (state->menu_list.mode != PAUSE_MENU)
                    state->menu_list.mode = recv_packet->mode;
                state->client_game_index = recv_packet->game_index;
            } break;
            
            case CLOSE_CONNECTION: {
                closesocket(sock.handle);
                state->is_client = false;
                state->menu_list.mode = MAIN_MENU;
                add_onscreen_notification(&state->notifications, "Connection Closed");
            } break;
        }
    } else {
        logprint("draw_join_menu()", "Received no bytes\n");
    }
}

internal void
client_set_selected(QSock_Socket sock, bool8 selected[SELECTED_SIZE], u32 index) {
    Play_Nine_Packet packet = {};
    packet.type = SET_SELECTED;
    packet.game_index = index;
    platform_memory_copy(packet.selected, selected, sizeof(selected[0]) * SELECTED_SIZE);
    win32_send(sock, (const char *)&packet, sizeof(packet), 0);
}

internal void
client_set_name(QSock_Socket sock, const char *name) {
    Play_Nine_Packet packet = {};
    packet.type = SET_NAME;
    platform_memory_copy(packet.buffer, (void*)name, get_length(name));
    win32_send(sock, (const char *)&packet, sizeof(packet), 0);
}

internal void
client_close_connection(QSock_Socket sock) {
    Play_Nine_Packet packet = {};
    packet.type = CLOSE_CONNECTION;
    win32_send(sock, (const char *)&packet, sizeof(packet), 0);
    closesocket(sock.handle);
}