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
    THREAD server_handle;
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
    ADD_ALL_DRAW_SIGNALS,

    CLOSE_CONNECTION
};

struct Play_Nine_Packet {
    s8 bytes;
    u8 id;
    s8 type;
    u32 game_index;

    // what the server sends
    Game game;
    union {
        Draw_Signal signals[DRAW_SIGNALS_AMOUNT];
        Draw_Signal signal;
    };
    u8 signals_count;
    enum Menu_Mode mode;

    // what the clients send
    bool8 selected[GI_SIZE];
    char buffer[TEXTBOX_SIZE];
};

Play_Nine_Online online = {};

#endif // PLAY_NINE_ONLINE_H
