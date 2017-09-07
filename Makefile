
target:select_server epoll_server

select_server:select_server.c
	gcc -o $@ $^ -g -std=c99

epoll_server:epoll_server.c
	gcc -o $@ $^ -g -std=c99

clean:
	rm -rf $(target)

.PHONY:clean


