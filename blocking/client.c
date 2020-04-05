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
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
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
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    // THIS IS BLOCKING
    int n = write(sockfd,
        "everything is everything a cigarette for a wedding ring", 55);
    if (n < 0)
        error("ERROR writing to socket");
    char buffer[BUFFER_SIZE];
    // THIS IS BLOCKING
    n = read(sockfd, buffer, 255);
    if (n < 0)
        error("ERROR reading from socket");
    // THIS IS BLOCKING
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}
