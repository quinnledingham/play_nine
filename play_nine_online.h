struct Play_Nine_Online {
    QSock_Socket sock = {};
    QSock_Socket clients[5];
    u32 player_indices[5];
    u32 clients_index = 0;

    HANDLE server_handle; // thread handle
    HANDLE client_handles[5];
};

enum Packet_Types {
    SET_NAME,
    GET_GAME,
    SET_GAME,

    CLOSE_CONNECTION
};

#define BUFFER_SIZE sizeof(Game)

struct Play_Nine_Packet {
    u32 bytes;
    s32 type;
    enum Menu_Mode mode;
    char buffer[BUFFER_SIZE];
};

Play_Nine_Online online = {};

