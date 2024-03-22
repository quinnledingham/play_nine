struct Play_Nine_Online {
    QSock_Socket sock = {};
    QSock_Socket clients[5];
    u32 clients_index = 0;

    HANDLE server_handle; // thread handle
    HANDLE client_handles[5];
};

Play_Nine_Online online = {};

#define BUFFER_SIZE 1000

DWORD WINAPI
play_nine_client(void *parameters) {
    s32 bytes = win32_send(online.sock, 0, 0, 0);

    return 0;
}

DWORD WINAPI
play_nine_server_com(void *parameters) {
    char buffer[BUFFER_SIZE];

    while(1) {
        s32 bytes = win32_recv_from(online.clients[0], buffer, BUFFER_SIZE, 0, &online.sock.recv_info);
    };

    return 0;
}

DWORD WINAPI
play_nine_server(void *parameters) {
    State *state = (State *)parameters;
    
    if (qsock_server(&online.sock, state->port, TCP)) {
        state->menu_list.mode = LOCAL_MENU;
    } else {
        return 0;
    }

    while (1) {
        qsock_listen(online.sock);
        qsock_accept(&online.sock, &online.clients[online.clients_index]);

        DWORD thread_id;
        online.client_handles[online.clients_index] = CreateThread(0, 0, play_nine_server_com, (void*)state, 0, &thread_id);
        online.clients_index++;
    }

    return 0;
}