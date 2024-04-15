void linux_init_qsock() {

}

void linux_print_platform_error() {

}

s32 linux_select(struct QSock_Socket socket, int seconds, int microseconds) {
	return -1;
}

// Waits for a recieve on sock.
// returns zero on success   
// https://www.it-swarm-fr.com/fr/c/udp-socket-set-timeout/1070229989/

void linux_set_timeout(struct QSock_Socket socket, int seconds, int microseconds) {
	struct timeval tv;
	tv.tv_sec = seconds;
	tv.tv_usec = microseconds;
	
	// set socket option
	int success = setsockopt(socket.handle, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	if (success < 0) fprintf(stderr, "timeout(): error\n");
}

int linux_recv(struct QSock_Socket socket, const char *buffer, int buffer_size, int flags) {
	int bytes = recv(socket.handle, (void*)buffer, buffer_size, flags);
	if (bytes == -1) perror("linux_recv() error");
	return bytes;
}

int linux_recv_from(struct QSock_Socket socket, const char *buffer, int buffer_size, int flags, struct QSock_Address_Info *info) {
	int bytes = recvfrom(socket.handle, (void*)buffer, buffer_size, flags, (struct sockaddr*)info->address, &info->address_length);
	if (bytes == -1) perror("linux_recv_from() error");
	return bytes;
}

int linux_send(struct QSock_Socket socket, const char *buffer, int buffer_size, int flags) {
	int bytes = send(socket.handle, (void*)buffer, buffer_size, flags);
	if (bytes == -1) perror("linux_send() error");
	return bytes;
}

int linux_send_to(struct QSock_Socket socket, const char *buffer, int buffer_size, int flags, struct QSock_Address_Info info) {
	int bytes = sendto(socket.handle, (void*)buffer, buffer_size, flags, (struct sockaddr*)info.address, info.address_length);
	if (bytes == -1) perror("linux_send_to() error");
	return bytes;
}

void linux_listen(struct QSock_Socket socket) {
	int backlog = 5;
	int error = listen(socket.handle, backlog);
	if (error == -1) { perror("sock_listen() error"); }
}

void linux_accept(struct QSock_Socket *socket, struct QSock_Socket *client) {
	if (!socket->passive) {
		fprintf(stderr, "linux_accept(): not a passive(server) socket\n");
		return;
	}
	else if (socket->protocol != TCP) {
		fprintf(stderr, "linux_accept(): not a TCP socket\n");
		return;
	}
	
	struct sockaddr address = {};
	unsigned int address_length;

	client->handle = accept(socket->handle, &address, &address_length);
	if (client->handle == -1) fprintf(stderr, "linux_accept(): accept() call failed\n");

	client->info.address = (const char *)malloc(address_length);
	memcpy((void*)client->info.address, (void*)&address, address_length);
	client->info.address_length = address_length;	
}

const char* linux_get_ip(struct QSock_Address_Info info) {
	struct sockaddr_in *c = (struct sockaddr_in *)info.address;
	char *ip = (char *)malloc(80);
	inet_ntop(info.family, &(c->sin_addr), ip, 80);
	return ip;
}

const char* linux_get_ip_from_web(const char *webname) {
	printf("address: %s\n", webname);

	struct addrinfo hints, *info;
	memset(&hints, 0, sizeof(struct addrinfo));
	info = (struct addrinfo*)malloc(sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(webname, NULL, &hints, &info)) perror("linux_get_ip_from_web() getaddrinfo error\n");

    struct sockaddr_in *address = (struct sockaddr_in *)info->ai_addr;
    char *ip = (char *)malloc(200);
    memset(ip, 0, 200);
   	const char *dst = inet_ntop(info->ai_family, &(address->sin_addr), ip, 200);
   	if (dst == 0) perror("inet_ntop error");

   	printf("ip: %s\n", ip);
   	return ip;

	/*
    struct QSock_Address_Info address_info = {};
    address_info.family = info->ai_family;
    address_info.address = (void*)info->ai_addr;
    printf("YO: %p\n", address_info.address);
    return qsock_get_ip(address_info);
    */
}

internal struct QSock_Address_Info
linux_addrinfo_to_address_info(struct addrinfo og) {
	struct QSock_Address_Info info = {};
	info.family = og.ai_family;
	info.socket_type = og.ai_socktype;
	info.protocol = og.ai_protocol;
	info.address_length = og.ai_addrlen;

	info.address = (const char *)malloc(info.address_length + 1);
	memset((void*)info.address, 0, info.address_length + 1);
	memcpy((void*)info.address, og.ai_addr, info.address_length);

	return info;
}

// find a address for the socket that works
// ip == NULL for passive to get wildcard address
bool8 linux_init_socket(struct QSock_Socket *sock, const char *ip, const char *port) {
	struct addrinfo hints = {};
	hints.ai_family = AF_INET; //IPv4
	if (sock->passive) hints.ai_flags = AI_PASSIVE; // returns address for bind/accept
    //hints.ai_protocol = 0;

	switch(sock->protocol)
	{
		case TCP: hints.ai_socktype = SOCK_STREAM; hints.ai_protocol = IPPROTO_TCP; break;
		case UDP: hints.ai_socktype = SOCK_DGRAM;  hints.ai_protocol = IPPROTO_UDP; break;
	}

	struct addrinfo *info = (struct addrinfo*)malloc(sizeof(struct addrinfo));
	if (getaddrinfo(ip, port, &hints, &info)) fprintf(stderr, "getaddrinfo error");

	// find address that works looping through addrinfo linked list
	struct addrinfo *ptr;
	for (ptr = info; ptr != NULL; ptr = ptr->ai_next) {
		sock->handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		
		if (sock->handle == -1) {
			perror("linux_socket(): socket() call failed");
			continue;
		}

		if (sock->passive) {
			// bind socket to address if passive socket everytime
			if (bind(sock->handle, ptr->ai_addr, ptr->ai_addrlen) != -1) break;
			perror("get_address_info(): bind() call failed");
		} else {
			// connect socket to address if not passive socket only when using TCP
			if (sock->protocol == UDP || connect(sock->handle, ptr->ai_addr, ptr->ai_addrlen) != -1) break;
			perror("get_address_info(): connect() call failed");
		}
	}

	if (sock->passive)
		sock->info = linux_addrinfo_to_address_info(*ptr);
	else
		sock->connected = linux_addrinfo_to_address_info(*ptr);

	freeaddrinfo(info);

	return true;
}

void linux_free_socket(struct QSock_Socket socket) {
	shutdown(socket.handle, SHUT_RDWR);
	close(socket.handle);
}
