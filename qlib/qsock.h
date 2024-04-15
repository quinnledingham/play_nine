#ifndef QSOCK_H
#define QSOCK_H

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
	// WARNING: these are set to the values from the platforms enums, not the above enums
	s32 family;
	s32 socket_type;
	s32 protocol;

	u32 address_length;
	const char *address; // sockaddr (socket address)
};

struct QSock_Socket {
	s32 handle; // linux: file descriptor
	enum QSock_Protocol protocol; // TCP or UDP
	bool32 passive; // if true it waits for connection requests to come in (ie server)

	struct QSock_Address_Info info; // info about the address of this socket
	struct QSock_Address_Info connected; // client: info about address of server connected to, server: info about server address
};

#ifdef OS_WINDOWS

#define OS_EXT(n) win32_##n

#elif OS_LINUX

#define OS_EXT(n) linux_##n

#endif // OS

#define QSOCK_FUNC(r, n, ...) r OS_EXT(n)(__VA_ARGS__); r (*qsock_##n)(__VA_ARGS__) = &OS_EXT(n)

QSOCK_FUNC(void, init_qsock, );
QSOCK_FUNC(bool8, init_socket, struct QSock_Socket *sock, const char *ip, const char *port);
QSOCK_FUNC(void, print_platform_error, );
QSOCK_FUNC(s32, select, struct QSock_Socket sock, s32 seconds, s32 microseconds);
QSOCK_FUNC(void, listen, struct QSock_Socket socket);
QSOCK_FUNC(void, accept, struct QSock_Socket *sock, struct QSock_Socket *client);
QSOCK_FUNC(s32, send_to,   struct QSock_Socket socket, const char *buffer, int buffer_size, int flags, struct QSock_Address_Info info);
QSOCK_FUNC(s32, recv_from, struct QSock_Socket socket, const char *buffer, int buffer_size, int flags, struct QSock_Address_Info *info);
QSOCK_FUNC(void, free_socket, struct QSock_Socket socket);

internal bool8
qsock_client(struct QSock_Socket *sock, const char *ip, const char *port, enum QSock_Protocol protocol) {
	sock->protocol = protocol;
	sock->passive = false;
	if (qsock_init_socket(sock, ip, port)) {

		//if (sock->protocol == UDP)
			//qsock_set_timeout(*sock, 1, 0);

		printf("Client socket connected to ip: %s port: %s\n", ip, port);
		return true;
	} else {
		qsock_print_platform_error();
		return false;
	}
}

internal bool8
qsock_server(struct QSock_Socket *sock, const char *port, enum QSock_Protocol protocol) {
	sock->protocol = protocol;
	sock->passive = true;
	if (qsock_init_socket(sock, NULL, port)) {

		printf("Server socket set up with port %s\n", port);
		return true;
	} else {
		qsock_print_platform_error();
		return false;
	}
}

// Do proper send and recv for each protocol
// if connected == NULL use sock.connected address
internal s32
qsock_recv(struct QSock_Socket sock, struct QSock_Socket *connected, const char *buffer, u32 buffer_size) {
	QSock_Socket sock_to_use;
	QSock_Address_Info send_address;

	if (connected != NULL) {
		if (connected->info.address == 0) {
	    connected->info.address = (const char *)malloc(sizeof(sockaddr));
	    connected->info.address_length = sizeof(sockaddr);
	    connected->info.family = (QSock_Family)AF_INET;
		}
	
		send_address = connected->info;
	} else {
		send_address = sock.connected;
	}

	if (sock.passive && sock.protocol == TCP) {
		sock_to_use = *connected;
	} else {
		sock_to_use = sock;
	}
		
	s32 bytes = qsock_recv_from(sock_to_use, buffer, buffer_size, 0, &send_address);
	return bytes;
}

internal s32
qsock_send(struct QSock_Socket sock, struct QSock_Socket *connected, const char *buffer, u32 buffer_size) {
	QSock_Socket sock_to_use;
	QSock_Address_Info send_address;

	if (connected != NULL) {
		send_address = connected->info;
	} else {
		send_address = sock.connected;
	}

	if (sock.passive && sock.protocol == TCP) {
		sock_to_use = *connected;
	} else {
		sock_to_use = sock;
	}

	s32 bytes = qsock_send_to(sock_to_use, buffer, buffer_size, 0, send_address);
	return bytes;
}

#ifdef OS_WINDOWS

#include "win32_qsock.c"

#elif OS_LINUX

#include "linux_qsock.c"

#endif // OS

#endif // QSOCK_H
