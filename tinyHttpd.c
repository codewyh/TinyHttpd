
#include <stdint.h>
#include "epoll.h"
#include "threadpool.h"
#include "util.h"
#include "log.h"
#include "httpd.h"

extern struct epoll_event *events;

int main(int argc, char **argv)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	if(sigaction(SIGPIPE, &sa, NULL)) {
		LOG_ERR("SIGPIPE ERR");
		return 0;
	}

	int listenfd;
	listenfd = open_listenfd(8888);

	make_socket_non_blocking(listenfd);

	int epfd = tiny_epoll_create(0);
	struct epoll_event event;
	event.data.fd = listenfd;
	event.events = EPOLLIN | EPOLLET;
	tiny_epoll_add(epfd, listenfd, &event);

	tiny_threadpool_t *tp = threadpool_init(2);

	while(1) {
		int n = tiny_epoll_wait(epfd, events, MAXEVENTS, -1);
		int i = 0;
		for(;i < n;++i){
			if (listenfd == events[i].data.fd) {
				while(1) {
					struct sockaddr_in clientaddr;
					socklen_t inlen = sizeof(clientaddr);

					int infd = accept(listenfd, (struct sockaddr*)&clientaddr,&inlen);
					if (infd == -1) {
						if((errno == EAGAIN) || (errno == EWOULDBLOCK))
							break;
						else{
							LOG_ERR("accept err");
							break;
						}
					}
					LOG_INFO("new client fd : %d",infd);

					make_socket_non_blocking(infd);

					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					tiny_epoll_add(epfd, infd, &event);
				}
			}
			else {
				if((events[i].events & EPOLLERR)
						|| (events[i].events & EPOLLHUP)
						|| !(events[i].events & EPOLLIN)) {
					LOG_ERR("epoll error fd : %d",events[i].data.fd);
					close(events[i].data.fd);
					continue;
				}

				LOG_INFO("new task from fd : %d", events[i].data.fd);
				threadpool_add(tp,accept_request,&(events[i].data.fd));
			}
		}
	}
	LOG_INFO("begin destroy");
	if(threadpool_destroy(tp,1) < 0)
		LOG_ERR("idestroy err");

	return 0;

}
