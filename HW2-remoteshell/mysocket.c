#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "util.h"
#include "mysocket.h"

int build_server_socket(int * listen_sock, const int port, const char * failMessage){
    int lis_sock = *listen_sock;
    if(lis_sock < 0) {
        failHandler(failMessage);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(bind(lis_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        failHandler(failMessage);
    }
    if(listen(lis_sock, 1) < 0){
        failHandler(failMessage);
    }
    int addr_len = sizeof(addr);
    int server_sock = accept(lis_sock, (struct sockaddr *) &addr, (socklen_t *) &addr_len);
    if(server_sock < 0) {
        failHandler(failMessage);
    }

    return server_sock;
}

int build_client_socket(const char * ip_addr, const int port, const char * failMessage){
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(client_sock < 0) {
        failHandler(failMessage);
    }

    struct sockaddr_in addr; 
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip_addr);
    addr.sin_port = htons(port);

    if(connect(client_sock, (struct sockaddr *) &addr, sizeof(addr)) < 0){
        failHandler(failMessage);
    }

    return client_sock;
}
