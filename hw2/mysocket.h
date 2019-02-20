
#ifndef _MYSOCK_H_
#define _MYSOCK_H_

#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE     1024
struct my_socket_package{
    int length;
    int is_end;
    char message[BUFFER_SIZE];
};

int init_socket(const char * failMessage);
struct sockaddr_in init_address(in_addr_t address, const int port);
void server_socket_bind_listen(int lis_sock, struct sockaddr * addr, int max_clients, const char * failMessage);
int server_socket_accept(int lis_sock, struct sockaddr * addr, const char * failMessage);
void client_socket_connect(int cli_sock ,struct sockaddr * addr, const char * failMessage);

 #endif