#ifndef _EPOLL_H_
#define _EPOLL_H_

#include <sys/epoll.h>

#define MAXEVENTS 1024

int tiny_epoll_create(int flags);
void tiny_epoll_add(int epfd, int fd, struct epoll_event *event);
void tiny_epoll_mod(int epfd, int fd, struct epoll_event *event);
void tiny_epoll_del(int epfd, int fd, struct epoll_event *event);
int tiny_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);

#endif //_EPOLL_H_
