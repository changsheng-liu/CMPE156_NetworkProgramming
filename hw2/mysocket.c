#include <unistd.h>
#include <string.h>
#include "util.h"
#include "mysocket.h"


int init_socket(const char * failMessage){
    int tcpsocket = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpsocket < 0) {
        failHandler(failMessage);
    }
    return tcpsocket;
}

struct sockaddr_in init_address(in_addr_t address, const int port) {
    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = address;
    addr.sin_port = htons(port);
    return addr;
}

void server_socket_bind_listen(int lis_sock, struct sockaddr * addr, int max_clients, const char * failMessage) {
    if(bind(lis_sock, addr, sizeof(*addr)) < 0){
        failHandler(failMessage);
    }
    if(listen(lis_sock, max_clients) < 0){
        failHandler(failMessage);
    }
}

int server_socket_accept(int lis_sock, struct sockaddr * addr, const char * failMessage) {
    int addr_len = sizeof(*addr);
    int server_sock = accept(lis_sock, addr, (socklen_t *) &addr_len);
    if(server_sock < 0) {
        failHandler(failMessage);
    }

    return server_sock;
}

void client_socket_connect(int cli_sock ,struct sockaddr * addr, const char * failMessage) {
    if(connect(cli_sock, addr, sizeof(*addr)) < 0){
        failHandler(failMessage);
    }
}

