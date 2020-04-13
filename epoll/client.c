// TCP socket echo server using epoll
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    const int PORTNO = 0xC05;
    const int BUFFER_SIZE = 256;
    const char * ADDRESS = "127.0.0.1";
    // internet domain, stream socket, TCP
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    // argument is ignored but must be > 0
    int epollfd = epoll_create(1);
    if (epollfd < 0)
        error("ERROR creating epoll");
    struct epoll_event sock_event = {
        EPOLLIN | EPOLLOUT,
        NULL,
    };
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &sock_event);
    // BLOCK: this may be blocking??
    // looks hard to get around without being GNU libc specific (getaddrinfo_a)
    struct hostent * server = gethostbyname(ADDRESS);
    if (server == NULL) {
        error("ERROR no such host");
    };
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *) &serv_addr.sin_addr.s_addr,
          server->h_length);
    // network byte order
    serv_addr.sin_port = htons(PORTNO);
    int res;
    // epoll connect notifies when EPOLLOUT / write opens up
    res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    if (res < 0 && errno != EINPROGRESS) {
        error("ERROR on connect");
    }
    char * msg = "everything is everything a cigarette for a wedding ring";
    const int MSG_LEN = 55;
    int sent = 0;
    char buffer[BUFFER_SIZE];
    const int LEN = 255;
    int recieved = 0;
    while (sent < MSG_LEN || recieved < MSG_LEN) {
        const int MAX_EVENTS = 2;
        struct epoll_event events[MAX_EVENTS];
        int num_ready = epoll_wait(epollfd, events, 1, -1);
        if (num_ready < 0) {
            error("EPOLL error");
        }
        for (int i=0; i<1; i++) {
            if (events[i].events & EPOLLIN) {
                // read
                int res = read(sockfd, buffer + recieved, LEN - recieved);
                if (res < 0) {
                    error("ERROR reading");
                }
                recieved += res;
            }
            if (events[i].events & EPOLLOUT) {
                // write
                int res = write(sockfd, msg + sent, MSG_LEN - sent);
                // b/c epoll, res < 0 is def a real error
                if (res < 0) {
                    error("ERROR writing");
                }
                sent += res;
            }
        }
    }
    // THIS IS BLOCKING
    printf("%s\n", buffer);
    // not sure the proper order for this but this seems okay
    close(epollfd);
    close(sockfd);
    return 0;
}
