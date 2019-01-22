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
    if(argc != 2) {
        fprintf(stderr, "Please use server by correct input!\n");
        exit(-1);
    }

    if(!isValidPort(argv[1])) {
        fprintf(stderr, "Please use legal port number!\n");
        exit(-1);
    }

    //build server socket and listen socket
    int lis_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(lis_sock < 0) {
        fprintf(stderr, "Server Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));

    if(bind(lis_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "Sever Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }
    if(listen(lis_sock, 1) < 0){
        fprintf(stderr, "Sever Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }
    int addr_len = sizeof(addr);
    int server_sock = accept(lis_sock, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    if(server_sock < 0) {
        fprintf(stderr, "Sever Fail: Cannot establish socket connection. End of client.\n");
        exit(-1);
    }



    close(server_sock);

    return 0;
}
