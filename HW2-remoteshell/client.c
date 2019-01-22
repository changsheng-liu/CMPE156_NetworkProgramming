#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"

int main(int argc, char const *argv[])
{
    // check param is legal
    if(argc != 3) {
        fprintf(stderr, "Please use client by correct input!\n");
        exit(-1);
    }

    if(!isValidIP(argv[1])) {
        fprintf(stderr, "Please use legal ip address!\n");
        exit(-1);
    }

    if(!isValidPort(argv[2])) {
        fprintf(stderr, "Please use legal port number!\n");
        exit(-1);
    }

    // build client socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0) {
        fprintf(stderr, "Client Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    if(connect(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "Client Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }


    close(sock);

    return 0;
}
