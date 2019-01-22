#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"

int build_server_socket(const int port, const char * failMessage){
    int lis_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(lis_sock < 0) {
        fprintf(stderr, "%s\n", failMessage);
        exit(-1);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(lis_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "%s\n", failMessage);
        exit(-1);
    }
    if(listen(lis_sock, 1) < 0){
        fprintf(stderr, "%s\n", failMessage);
        exit(-1);
    }
    int addr_len = sizeof(addr);
    int server_sock = accept(lis_sock, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    if(server_sock < 0) {
        fprintf(stderr, "%s\n", failMessage);
        exit(-1);
    }

    return server_sock;
}

int build_client_socket(const char * ip_addr, const int port, const char * failMessage){
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock < 0) {
        fprintf(stderr, "%s\n" , failMessage);
        exit(-1);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    if(connect(client_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        fprintf(stderr, "%s\n" , failMessage);
        exit(-1);
    }

    return client_sock;
}
