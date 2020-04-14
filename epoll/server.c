// simple TCP socket echo server
// ultimate goal is to convert to use epoll
// almost entirely written from
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define PORTNO 0xC05
#define BUFFER_SIZE 256
#define MAX_EVENTS 8

int sockfd = -1;

void close_sock() {
    close(sockfd);
}
void cleanup() {
    close_sock();
    exit(0);
}

void error(char *msg) {
    perror(msg);
    close_sock();
    exit(1);
}

typedef struct conn_status {
    int fd;
    int begin;
    int end;
    // really a botched queue
    char buffer[BUFFER_SIZE];
} conn_status;

int main() {
    // internet domain, stream socket, TCP
    sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // i've just gotten tired of losing my port to a dangled socket
    signal(SIGINT, cleanup);
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    // network byte order
    serv_addr.sin_port = htons(PORTNO);
    // 0.0.0.0
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    // , size of backlog queue. 5 = max on most systems
    // this cannot fail
    listen(sockfd, 5);
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);
    int newsockfd;
    // argument is ignored but must be > 0
    int epollfd = epoll_create(1);
    if (epollfd < 0)
        error("ERROR creating epoll");
    struct epoll_event sock_event = {
        // accept() goes under IN
        EPOLLIN,
        // examples won't do this but `man epoll_ctl` says this can be an int
        (epoll_data_t) sockfd,
    };
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &sock_event);
    // i guess we accept multiple conns now cause it's just as easy
    while (1) {
        struct epoll_event events[MAX_EVENTS];
        int num_ev = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (num_ev < 0) {
            error("ERROR epoll_wait");
        }
        for (int i=0; i<num_ev; ++i) {
            if (events[i].data.fd == sockfd) {
                int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
                if (newsockfd < 0) {
                    error("ERROR on accept");
                }
                int flags = fcntl(newsockfd, F_GETFL, 0);
                fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);
                // TODO: why doesn't this drop at the end of this scope and
                // invalidate the pointer? im not complaining tho
                conn_status new_conn = {
                    newsockfd,
                    0,
                    0
                };
                // epoll now handles our list of connections
                struct epoll_event sock_event = {
                    // echo
                    EPOLLIN | EPOLLOUT | EPOLLRDHUP,
                    &new_conn,
                };
                epoll_ctl(epollfd, EPOLL_CTL_ADD, newsockfd, &sock_event);
            } else {
                // then it's a connection, not accept
                conn_status * conn = events[i].data.ptr;
                int res;
                if (events[i].events & EPOLLRDHUP) {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn->fd, NULL);
                }
                if (events[i].events & EPOLLIN) {
                    res = read(conn->fd, conn->buffer + conn->end, BUFFER_SIZE - conn->end);
                    if (res < 0) {
                        error("ERROR on read");
                    }
                    conn->end += res;
                }
                if (events[i].events & EPOLLOUT) {
                    printf("%d %d %d\n", conn->fd, conn->begin, conn->end);
                    res = write(conn->fd, conn->buffer + conn->begin, conn->end - conn->begin);
                    if (res < 0) {
                        error("ERROR on write");
                    }
                    conn->begin += res;
                }
                int same = conn->begin == conn->end;
                if (conn->end == BUFFER_SIZE || same) {
                    conn->end = 0;
                }
                if (conn->begin == BUFFER_SIZE || same) {
                    conn->begin = 0;
                }
            }
        }
    }
    close_sock();
    return 0;
}
