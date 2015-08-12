#include "epoll.h"
#include <stdio.h>
#include <stdlib.h>



struct epoll_event *events;

int tiny_epoll_create(int flags){
	int fd = epoll_create1(flags);
	if(fd < 0){
		fprintf(stderr,"epoll create error\n");
		exit(1);
	}
	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);
	return fd;
}

void tiny_epoll_add(int epfd, int fd, struct epoll_event *event){
	if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event)){
		fprintf(stderr,"epoll add error");
		exit(1);
	}
}

void tiny_epoll_mod(int epfd, int fd, struct epoll_event *event){
	if(epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event)){
		fprintf(stderr,"epoll mod error");
		exit(1);
	}
}

void tiny_epoll_del(int epfd, int fd, struct epoll_event *event){
	if(epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event)){
		fprintf(stderr,"epoll del error");
		exit(1);
	}
}

int tiny_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout){
	int n = epoll_wait(epfd, events, maxevents, timeout);
	if(n  < 0){
		fprintf(stderr,"epoll wait error");
		exit(1);
	}
	return n;
}
