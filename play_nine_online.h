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
    MUTEX mutex;
};

enum Packet_Types {
    CONNECT,

    SET_NAME,
    GET_GAME,
    SET_GAME,
    SET_SELECTED,

    CLOSE_CONNECTION
};

struct Play_Nine_Packet {
    u32 bytes;
    s32 type;
    u32 game_index;

    union {
        // what the server sends
        struct {
            enum Menu_Mode mode;
            Game game;
            Draw_Signal draw_signals[DRAW_SIGNALS_AMOUNT];
        };

        // what the clients send
        struct {
            bool8 selected[SELECTED_SIZE];
            char buffer[TEXTBOX_SIZE];
        };
    };
};

Play_Nine_Online online = {};

#endif // PLAY_NINE_ONLINE_H
