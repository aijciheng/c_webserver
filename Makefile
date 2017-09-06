
target:select_server

select_server:select_server.c
	gcc -o $@ $^ -g -std=c99

.PHONY:clean
