enum QSock_Family {
    INET,
    INET6
};

enum QSock_Socket_Type {
    STREAM,   // TCP
    DATAGRAMS // UDP
};

enum QSock_Protocol {
    TCP,
    UDP
};

#define TCP_BUFFER_SIZE 65536

struct QSock_Address_Info {
    s32 family;
    s32 socket_type;
    s32 protocol;

    const char *address;
    u32 address_length;
};

struct QSock_Socket {
    s32 handle;
    s32 protocol;

    QSock_Address_Info info; // client: info about address of server connected to, server: info about server address
    QSock_Address_Info recv_info; // most recent address that was received from (UDP)

    bool32 passive; // if true it waits for connection requests to come in (ie server)
    QSock_Socket *other; // filled in qsock_accept(). it is the sending sock for server using TCP
};

internal void
win32_init_winsock() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        print("WSAStartup failed with error: %d\n", iResult);
    }
}

internal struct QSock_Address_Info
addrinfo_to_address_info(struct addrinfo og) {
    struct QSock_Address_Info info = {};
    info.family = og.ai_family;
    info.socket_type = og.ai_socktype;
    info.protocol = og.ai_protocol;
    info.address_length = (u32)og.ai_addrlen;

    info.address = (const char *)malloc(info.address_length + 1);
    memset((void*)info.address, 0, info.address_length + 1);
    memcpy((void*)info.address, og.ai_addr, info.address_length);

    return info;
}

internal bool8
win32_init_socket(QSock_Socket *sock, const char *ip, const char *port) {
    addrinfo hints = {};
    hints.ai_family = AF_INET; // IPv4
    if (sock->passive)
        hints.ai_flags = AI_PASSIVE;

    switch(sock->protocol) {
        case TCP: hints.ai_socktype = SOCK_STREAM; break;
        case UDP: hints.ai_socktype = SOCK_DGRAM;  break;
    }

    addrinfo *info = (addrinfo*)malloc(sizeof(addrinfo));
    if (getaddrinfo(ip, port, &hints, &info)) {
        logprint("win32_init_socket()", "getaddrinfo error\n");
        return false;
    }

    addrinfo *ptr;
    for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {
        sock->handle = (s32)socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sock->handle == -1) {
            logprint("win32_init_socket()", "socket() call failed\n");
            continue;
        }

        if (sock->passive) {
            if (bind(sock->handle, ptr->ai_addr, (int)ptr->ai_addrlen) != -1) 
                break;
            logprint("win32_init_socket()", "bind() call failed\n");
            return false;
        } else {
            if (sock->protocol == UDP || connect(sock->handle, ptr->ai_addr, (int)ptr->ai_addrlen) != -1) 
                break;
            logprint("win32_init_socket()", "connect() call failed\n");
            return false;
        }
    }

    sock->info = addrinfo_to_address_info(*ptr);
    sock->other = (struct QSock_Socket*)malloc(sizeof(struct QSock_Socket));

    freeaddrinfo(info);

    return true;
}

internal s32
win32_timeout(QSock_Socket sock, s32 seconds, s32 microseconds) {
    // Waits for a recieve on sock. if none it returns 0.
    fd_set fds;
    int n;
    struct timeval tv;
    
    FD_ZERO(&fds);
    FD_SET(sock.handle, &fds);
    
    tv.tv_sec = seconds;
    tv.tv_usec = microseconds;
    
    n = select(sock.handle, &fds, NULL, NULL, &tv);
    if (n == 0)
        return 1; // Timeout
    else if (n < 0) {
        logprint("Timeout()", "select() call failed!\n");
        return 0; // Timeout Failed
    }
    
    return 0; //No Timeout
}

internal void
qsock_listen(struct QSock_Socket socket) {
    int backlog = 5;
    int error = listen(socket.handle, backlog);
    if (error == -1) { 
        print("sock_listen() error\n"); 
        print("WSAerror %d\n", WSAGetLastError());
    }
}

internal void
qsock_accept(struct QSock_Socket *socket, struct QSock_Socket *client) {
    if (!socket->passive) {
        fprintf(stderr, "qsock_accept(): not a passive(server) socket\n");
        return;
    }
    else if (socket->protocol != TCP) {
        fprintf(stderr, "qsock_accept(): not a TCP socket\n");
        return;
    }

    struct sockaddr address = {};
    s32 address_length = sizeof(sockaddr);

    client->handle = (s32)accept(socket->handle, &address, &address_length);
    if (client->handle == -1) {
        logprint("qsock_accept()", "accept() call failed\n");
        print("WSAerror %d\n", WSAGetLastError());
        return;
    }

    client->info.address = (const char *)malloc(address_length);
    memcpy((void*)client->info.address, (void*)&address, address_length);
}

internal bool8
qsock_client(QSock_Socket *sock, const char *ip, const char *port, int protocol) {
    sock->protocol = protocol;
    sock->passive = false;
    if (win32_init_socket(sock, ip, port)) {

        if (sock->protocol == UDP) 
            win32_timeout(*sock, 1, 0);

        print("Client socket connected to ip: %s port: %s\n", ip, port);
        return true;
    } else {

        print("WSAerror %d\n", WSAGetLastError());
        return false;
    }
}

internal bool8
qsock_server(QSock_Socket *sock, const char *port, s32 protocol) {
    sock->protocol = protocol;
    sock->passive = true;
    if (win32_init_socket(sock, NULL, port)) {

        print("Server socket set up with port %s\n", port);
        return true;
    } else {
        print("WSAerror %d\n", WSAGetLastError());
        return false;
    }
}