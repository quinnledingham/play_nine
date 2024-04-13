#ifndef PLAY_NINE_ONLINE_H
#define PLAY_NINE_ONLINE_H

struct Online_Player {
    QSock_Socket sock;
    s64 thread_handle;
    u32 game_index;
    bool8 in_use;

    State *state;
};

struct Play_Nine_Online {
    QSock_Socket sock = {};
    HANDLE server_handle; // thread handle
    HANDLE client_handle;

    Online_Player players[5];

    bool8 close_threads;
    s64 mutex;
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
            Rotation pile_rotation;
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