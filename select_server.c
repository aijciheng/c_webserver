#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

int accept_conn(int listenfd) {
    int fd;
    fd = accept(listenfd, 0, 0);
    if (fd == -1) {
        perror("accept fd error!!!\n");
    }
    return fd;
}

int query_conn(int connfd) {
    char buf[1024]; 
    int count = read(connfd, buf, sizeof(buf));
    if (count <= 0) {
        fprintf(stdout, "close connfd\n");
        close(connfd);
        return -1;
    } else {
        buf[count] = '\0';
        fprintf(stdout, "%s", buf);
        return 0;
    }

}

int main() {
    // init
    int listenfd, maxfd, connfd;
    struct sockaddr_in server_socket;
    fd_set rset, wset, _rset, _wset;
    int retval;
        
    // socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket is error!!!\n");
        exit(-1);
    }

    // bind
    server_socket.sin_family = AF_INET;
    server_socket.sin_port = htons(8088);
    server_socket.sin_addr.s_addr = 0;
    if (bind(listenfd, (struct sockaddr*)&server_socket, sizeof(struct sockaddr)) == -1) {
        perror("bind is error!!!\n");
        exit(-1);
    }

    // listen
    if (listen(listenfd, 1024) == -1) {
        perror("listen is error!!!\n");
        exit(-1);
    }

    // select
    FD_ZERO(&rset);
    FD_ZERO(&wset);
    FD_SET(listenfd, &rset);
//    FD_SET(listenfd, wset);
    maxfd = listenfd;
    while (1) {
        memcpy(&_rset, &rset, sizeof(fd_set)); 
        memcpy(&_wset, &wset, sizeof(fd_set));
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 5 * 1000 * 1000; // 5s
        retval = select(maxfd + 1, &_rset, &_wset, 0, &tv); 
        if (retval == -1) {
            perror("select error!");
        } else if (retval) {
            fprintf(stdout, "data has come!!!\n");
            for (int fd = 0; fd <= maxfd; fd++) {
                if (FD_ISSET(fd, &_rset) || FD_ISSET(fd, &_wset)) {
                    if (fd == listenfd) {
                        // do accept
                        connfd = accept_conn(fd);
                        if (connfd != -1) {
                            FD_SET(connfd, &rset);
                            //FD_SET(connfd, wset);
                            if (connfd > maxfd) {
                                maxfd = connfd;
                            }
                        }
                    } else {
                        // do business
                        if (query_conn(fd) == -1) {
                            FD_CLR(fd, &rset);
                        }
                    }
                }
            }
        } else {
            fprintf(stdout, "no data has come, please check!!!\n");
        }
    }
    
    // close
    return 0;
}
