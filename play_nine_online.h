#ifndef PLAY_NINE_ONLINE_H
#define PLAY_NINE_ONLINE_H

struct Online_Player {
    QSock_Socket sock;
    THREAD thread_handle;
    u32 game_index;
    bool8 in_use;

    State *state;
};

struct Play_Nine_Online {
    QSock_Socket sock = {};
    THREAD server_handle; // thread handle
    THREAD client_handle;

    Online_Player players[5];

    bool8 close_threads;
};

enum Packet_Types {
    CONNECT,

    SET_NAME,
    SET_GAME,
    SET_SELECTED,
    SET_MENU_MODE,
    ADD_DRAW_SIGNAL,

    CLOSE_CONNECTION
};

struct Play_Nine_Packet {
    u32 bytes;
    s32 type;
    u32 game_index;

    // what the server sends
    Game game;
    Draw_Signal signal;
    enum Menu_Mode mode;

    // what the clients send
    bool8 selected[SELECTED_SIZE];
    char buffer[TEXTBOX_SIZE];
};

Play_Nine_Online online = {};

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
server_send_menu_mode(enum Menu_Mode mode) {
    Play_Nine_Packet packet = {};
    packet.type = SET_MENU_MODE;
    packet.mode = mode;
    server_send_packet_all(&packet);
};

internal void
server_add_draw_signal(Draw_Signal signal) {
    Play_Nine_Packet packet = {};
    packet.type = ADD_DRAW_SIGNAL;
    packet.signal = signal;
    server_send_packet_all(&packet);
}

#endif // PLAY_NINE_ONLINE_H
