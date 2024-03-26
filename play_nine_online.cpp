#include "play_nine_online.h"

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

DWORD WINAPI
play_nine_client(void *parameters) {
    s32 bytes = win32_send(online.sock, 0, 0, 0);

    return 0;
}

DWORD WINAPI
play_nine_server_com(void *parameters) {
    State *state = (State *)parameters;
    char buffer[sizeof(Play_Nine_Packet)];

    u32 player_index = online.player_indices[online.clients_index];
    u32 client_index = online.clients_index;
    online.clients_index++;

    while(1) {
        s32 bytes = win32_recv_from(online.clients[client_index], buffer, sizeof(Play_Nine_Packet), 0, &online.sock.recv_info);
        if (bytes > 0) {
            Play_Nine_Packet *packet = (Play_Nine_Packet *)&buffer;
            switch(packet->type) {
                case SET_NAME: {
                    platform_memory_copy(state->game.players[player_index].name, packet->buffer, MAX_NAME_SIZE);
                } break;

                case GET_GAME: {
                    Play_Nine_Packet return_packet = {};
                    return_packet.type = SET_GAME;
                    if (state->menu_list.mode == PAUSE_MENU)
                        return_packet.mode = IN_GAME;
                    platform_memory_copy(return_packet.buffer, &state->game, sizeof(Game));
                    win32_send(online.clients[client_index], (const char *)&return_packet, sizeof(return_packet), 0);
                } break;

                case SET_GAME: {
                    win32_wait_mutex(state->mutex);
                    state->game = *(Game *)packet->buffer;
                    win32_release_mutex(state->mutex);
                } break;
            }
        }
    };

    return 0;
}

DWORD WINAPI
play_nine_server(void *parameters) {
    State *state = (State *)parameters;
    
    if (qsock_server(&online.sock, state->port, TCP)) {
        state->menu_list.mode = LOCAL_MENU;
        state->game.num_of_players = 1;
        default_player_name_string(state->game.players[0].name, 0);
    } else {
        return 0;
    }

    while (1) {
        qsock_listen(online.sock);
        qsock_accept(&online.sock, &online.clients[online.clients_index]);

        online.player_indices[online.clients_index] = state->game.num_of_players;

        win32_wait_mutex(state->mutex);

        state->game.num_of_players++;
        
        win32_release_mutex(state->mutex);

        DWORD thread_id;
        online.client_handles[online.clients_index] = CreateThread(0, 0, play_nine_server_com, (void*)state, 0, &thread_id);
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
        state->game = *(Game *)recv_packet->buffer;
        state->menu_list.mode = recv_packet->mode;
    } else {
        logprint("draw_join_menu()", "Received no bytes\n");
    }
}

internal void
client_set_game(QSock_Socket sock, Game *game) {
    Play_Nine_Packet packet = {};
    packet.type = SET_GAME;
    platform_memory_copy(packet.buffer, game, sizeof(Game));
    win32_send(sock, (const char *)&packet, sizeof(packet), 0);
}