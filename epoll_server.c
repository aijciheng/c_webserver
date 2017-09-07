#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>


#define MAX_CLIENT 1024

int accept_conn(int listenfd) {
    int fd = accept(listenfd, 0, 0);
    if (fd == -1) {
        if (errno != EINTR) {
            perror("accept failed:");
        }
    }
    return fd;
}

int query_conn(int fd) {
    char buf[1024];
    int count = read(fd, buf, sizeof(buf));
    if (count == -1) {
        perror("read failed:");
        close(fd);
        return -1;
    } else if (count == 0) {
        fprintf(stderr, "client close:");
        close(fd);
        return -1;
    } else {
        buf[count] = '\0';
        fprintf(stdout, "%s", buf);
        return 1;
    }
}

int main() {
    // init
    int listenfd, connfd, epollfd, retval, fd; 
    struct sockaddr_in server_socket;
    struct epoll_event client_events[MAX_CLIENT];
    
    // socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket failed:");
        exit(-1);
    }
    
    // bind
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(8088);
    server_socket.sin_addr.s_addr = 0;
    if (bind(listenfd, (struct sockaddr*)&server_socket, sizeof(struct sockaddr)) == -1) {
        perror("bind failed:");
        exit(-1);
    } 

    // listen
    if (listen(listenfd, 1024) == -1) {
        perror("listenfd failed:");
        exit(-1);
    }

    // epoll
    if ((epollfd = epoll_create(1024)) == -1) {
        perror("epoll create faile:");
        exit(-1);
    }

    struct epoll_event event = {0};
    event.events = EPOLLIN;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listenfd, &event) == -1) {
        perror("epoll ctl failed:");
        exit(-1);
    }
    while (1) {
        retval = epoll_wait(epollfd, client_events, MAX_CLIENT, 2000); 
        if (retval == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("epoll wait failed:");
            exit(-1);
        } else if (retval) {
            fprintf(stdout, "data has come\n");
            for (int i = 0; i < retval; i++) {
                if ((client_events[i].events & EPOLLIN) == EPOLLIN) {
                    fprintf(stdout, "epoll in\n");
                    fd = client_events[i].data.fd;
                    if (fd == listenfd) { 
                        // accept
                        connfd = accept_conn(fd);
                        event.events = 0;
                        event.events |= EPOLLIN;
                        if (connfd != -1) {
                            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) == -1) {
                                perror("epoll ctl failed:");
                                continue;
                            }
                        }
                    } else {
                        // do business
                        if (query_conn(fd) == -1) {
                            if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event) == -1) {
                                perror("epoll ctl failed:");
                            }
                        }
                    }
                }
            }
        } else {
            fprintf(stdout, "no data has come\n");
        }
    }
    return 0;
}
