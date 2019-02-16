
#ifndef _MYSOCK_H_
#define _MYSOCK_H_

#define BUFFER_SIZE     1024
#define COMMAND_SIZE 4

//protocol define 
//download command: "DWN <filename> <startbyteposition> <endbyteposition>"
#define CMD_DOWNLOAD "DWN" 
//check file command: "CHK <filename>"
#define CMD_CHECKFILE "CHK"
//acknowledge command: "ACK <integer>"
// #define CMD_ACK "ACK"

typedef struct{
    long file_length;
    int have_file_flag;
    int content_length;
    int content_is_end;
    char cmd[COMMAND_SIZE];
    char file_content[BUFFER_SIZE];
}server_response_t;

 #endif