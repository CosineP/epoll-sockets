// simple TCP socket echo server
// ultimate goal is to convert to use epoll
// almost entirely written from
// https://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html

#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <errno.h>
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
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
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
    do {
        res = connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    } while (res < 0);
    char * msg = "everything is everything a cigarette for a wedding ring";
    const int MSG_LEN = 55;
    int n = 0;
    do {
        int res = write(sockfd, msg + n, MSG_LEN - n);
        if (res < 0) {
            if (errno != EWOULDBLOCK) {
                error("ERROR writing");
            }
        } else {
            n += res;
        }
    } while (n < MSG_LEN);
    char buffer[BUFFER_SIZE];
    const int LEN = 255;
    n = 0;
    do {
        int res = read(sockfd, buffer + n, LEN - n);
        if (res < 0) {
            if (errno != EWOULDBLOCK) {
                error("ERROR reading");
            }
        } else {
            n += res;
        }
    } while (n < MSG_LEN);
    // THIS IS BLOCKING
    printf("%s\n", buffer);
    close(sockfd);
    return 0;
}
