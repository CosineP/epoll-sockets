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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

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

int main() {
    const int PORTNO = 0xC05;
    const int BUFFER_SIZE = 256;
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
    do {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    } while (newsockfd < 0);
    int flags = fcntl(newsockfd, F_GETFL, 0);
    fcntl(newsockfd, F_SETFL, flags | O_NONBLOCK);
    char buffer[BUFFER_SIZE];
    // bzero(buffer, 256); // i see no reason to do this if we have n
    // loop until we get a real socket error
    int rem_to_echo = 0;
    do {
        int res;
        do {
            res = read(newsockfd, buffer + rem_to_echo, BUFFER_SIZE - rem_to_echo);
        } while (res < 0 && errno == EWOULDBLOCK);
        if (res > 0) {
            rem_to_echo += res;
        }
        do {
            res = write(newsockfd, buffer, rem_to_echo);
        } while (res < 0 && errno == EWOULDBLOCK);
        if (res > 0) {
            rem_to_echo -= res;
        }
    // TODO: this doesn't really exit correctly, because even on broken pipe
    // errno == EWOULDBLOCK, i don't know why
    } while (errno == EWOULDBLOCK);
    close_sock();
    return 0;
}
