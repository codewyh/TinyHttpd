#include "util.h"

int open_listenfd(int port) {
	if (port <= 0){
		port = 8888;
	}

	int listenfd, optval = 1;
	struct sockaddr_in serveraddr;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
				(const void *)&optval, sizeof(int)) < 0)
		return -1;

	memset(&serveraddr, 0 ,sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	if(bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	if (listen(listenfd, LISTENQ) < 0)
		return -1;

	return listenfd;
}

int make_socket_non_blocking(int fd) {
	LOG_INFO("make non blocking  fd  :  %d",fd);
	int flags, s;
	flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1){
		LOG_ERR("fcntl 1");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl(fd, F_SETFL, flags);
	if (s == -1) {
		LOG_ERR("fcntl 2");
		return -1;
	}

	return 0;
}
