#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "util.h"
#include "mysocket.h"

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
    int client_sock = build_client_socket(argv[1], atoi(argv[2]), "Client Fail: Cannot establish socket connection. End of client.");

    close(client_sock);
    return 0;
}
