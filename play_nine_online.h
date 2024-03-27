struct Online_Player {
    QSock_Socket sock;
    s64 thread_handle;
    u32 game_index;
    bool8 in_use;

    State *state;
};

struct Online_Thread {
    s64 thread_handle;
    bool8 in_use;
};

struct Play_Nine_Online {
    QSock_Socket sock = {};
    HANDLE server_handle; // thread handle

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

#define BUFFER_SIZE sizeof(Game)

struct Play_Nine_Packet {
    u32 bytes;
    s32 type;
    u32 game_index;
    enum Menu_Mode mode;
    char buffer[BUFFER_SIZE];
    bool8 selected[SELECTED_SIZE];
};

Play_Nine_Online online = {};

