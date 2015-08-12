#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "log.h"

#define LISTENQ		1024

#define	BUFLEN		8192

#define	DELIM		"="

#define TINY_CONF_OK	0
#define TINY_CONF_ERROR	100

typedef struct {
	void *root;
	int port;
	int thread_num;
} tiny_conf_t;

int open_listenfd(int port);
int make_socket_non_blocking(int fd);

int read_conf(char *filename, tiny_conf_t *conf, char *buf, int len);

#endif
