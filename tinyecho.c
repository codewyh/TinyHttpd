#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <fcntl.h>

#include "util.h" 
#include "epoll.h"
#include "threadpool.h"
#include "rio.h"


extern struct epoll_event *events;

void echo(void *arg);

int main(int argc, char **argv) {
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE,&sa,NULL)){
		LOG_ERR("SIGPIPE err");
		return 0;
	}


	int listenfd;
	//struct sockaddr_in clientaddr;
	//socklen_t inlen = 1;
	//memset(&clientaddr, 0, sizeof(struct sockaddr_in));

	listenfd = open_listenfd(8888);

	int rc = make_socket_non_blocking(listenfd);
	assert(rc == 0);

	int epfd = tiny_epoll_create(0);
	struct epoll_event event;
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET;
	tiny_epoll_add(epfd, listenfd, &event);

	tiny_threadpool_t *tp = threadpool_init(2);

	while(1) {
		int n = tiny_epoll_wait(epfd, events, MAXEVENTS, -1);
		int i = 0;
		LOG_INFO("epoll return n : %d",n);
		for(;i < n; ++i){
			if( listenfd == events[i].data.fd) {
				while(1) {
					struct sockaddr_in clientaddr;
					socklen_t inlen = sizeof(clientaddr);

					int infd = accept(listenfd, (struct sockaddr *)&clientaddr, &inlen);
					if(infd == -1) {
						if((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
							break;
						}
						else{
							LOG_ERR("accept err");
							break;
						}
					}
					LOG_INFO("new  clientfd : %d",infd);

					make_socket_non_blocking(infd);

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;

					tiny_epoll_add(epfd, infd, &event);
				}
			}
			else {
				if ((events[i].events & EPOLLERR) ||
						(events[i].events & EPOLLHUP) ||
						!(events[i].events & EPOLLIN)) {
					LOG_ERR("epoll error fd : %d",events[i].data.fd);
					close(events[i].data.fd);
					continue;
				}


				LOG_INFO("new task from fd : %d", events[i].data.fd);
				threadpool_add(tp, echo, &(events[i].data.fd));
			}
		}
	}
	
	if (threadpool_destroy(tp,1) < 0)
		LOG_ERR("destroy err");

	return 0;
}

void echo(void *arg) {
	int connfd = *((int *)arg);
	size_t n;
	char buf[RIO_BUFSIZE];
	rio_t rio;

	rio_readinitb(&rio,connfd);

	while((n = rio_readlineb(&rio,buf,RIO_BUFSIZE)) != 0){
		if(n == -EAGAIN)
			return;
		printf("server receive %d bytes \n",n);
		rio_writen(connfd,buf,n);
	}
	close(connfd);
}
