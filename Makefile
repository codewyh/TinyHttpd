testhttp : tinyHttpd.c threadpool.c epoll.c rio.c util.c httpd.c
		gcc -g -Wall -o $@ $^ -lpthread

clean:
		rm testhttp

