
#ifndef _MYSOCK_H_
#define _MYSOCK_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>

#define BUFFER_SIZE     1024
#define COMMAND_SIZE 4
#define CMD_BYE "BYE"
#define CMD_DOWNLOAD "DWN"
#define CMD_CHECKFILE "CHK"

struct client_package{
    char cmd[COMMAND_SIZE];
    int start_prt;
    int end_prt;
    char file_name[BUFFER_SIZE];
};

struct server_package{
    char cmd[COMMAND_SIZE];
    int have_file_flag;
    long file_length;
    int content_length;
    int content_is_end;
    char file_content[BUFFER_SIZE];
};

int init_socket(const char * failMessage);
struct sockaddr_in init_address(in_addr_t address, const int port);
void server_socket_bind_listen(int lis_sock, struct sockaddr * addr, int max_clients, const char * failMessage);
int server_socket_accept(int lis_sock, struct sockaddr * addr, const char * failMessage);
void client_socket_connect(int cli_sock, struct sockaddr * addr, const char * failMessage);

void client_check_file(int conn_sock, const char * filename);
void client_download_file(int conn_sock, const char * filename, int start, int end);

void server_response_check_file(int conn_sock, const char * filename);
void server_upload_file(int conn_sock, const char * filename, int start, int end);

void socket_exit(int conn_sock);
 #endif