// simple TCP socket echo server
// ultimate goal is to convert to use epoll
// almost entirely written from
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(char *msg) {
    perror(msg);
    exit(1);
}

int main() {
    const int PORTNO = 0xC05;
    const int BUFFER_SIZE = 256;
    // internet domain, stream socket, TCP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
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
    int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    char buffer[BUFFER_SIZE];
    bzero(buffer, 256);
    // HERE is our blocking call!!!!
    int n = read(newsockfd, buffer, BUFFER_SIZE);
    if (n < 0) error("ERROR reading from socket");
    n = write(newsockfd, buffer, n);
    close(sockfd);
    return 0;
}
