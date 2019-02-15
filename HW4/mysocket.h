
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
    long start_prt;
    long end_prt;
    char cmd[COMMAND_SIZE];
    char file_name[BUFFER_SIZE];
};

struct server_package{
    long file_length;
    int have_file_flag;
    int content_length;
    int content_is_end;
    char cmd[COMMAND_SIZE];
    char file_content[BUFFER_SIZE];
};

 #endif