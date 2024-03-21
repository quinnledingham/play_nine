DWORD WINAPI
play_nine_server_com(void *parameters) {
    while(1) {

    };

    return 0;
}

DWORD WINAPI
play_nine_server(void *parameters) {
    State *state = (State *)parameters;

    QSock_Socket sock = {};
    QSock_Socket clients[5];
    u32 clients_index = 0;
    
    if (qsock_server(&sock, state->port, TCP)) {
        state->menu_list.mode = LOCAL_MENU;
    } else {
        return 0;
    }

    while (1) {
        qsock_listen(sock);
        qsock_accept(&sock, &clients[clients_index++]);

        DWORD thread_id;
        HANDLE thread_handle = CreateThread(0, 0, play_nine_server_com, (void*)state, 0, &thread_id);
        CloseHandle(thread_handle);
    }

    return 0;
}