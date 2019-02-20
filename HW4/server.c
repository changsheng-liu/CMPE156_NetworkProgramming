#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "myprotocol.h"
#include "util.h"

typedef struct{
    char target_file[BUFFER_SIZE];
    struct sockaddr_in clent_addr;
    long start;
    long end;
} thread_param_t;

void checkParam(int argc, char* argv[]);

void * upload(void * param) {
    // sleep(1);
    thread_param_t * thread_param = (thread_param_t *)param;
    struct sockaddr_in * clent_addr = &thread_param->clent_addr;
    char * target_file = thread_param->target_file;
    long start = thread_param->start;
    long end = thread_param->end;
    // create a new socket
    int server_fd;
    if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    char read_buf[BUFFER_SIZE];
    server_response_t * write_buf = malloc(sizeof(server_response_t));
    char * cmd;
    char data[BUFFER_SIZE];
    FILE *fp; 
    if((fp = fopen(target_file,"r")) == NULL) { 
        failHandler("open file error!");
    }
    struct sockaddr_in ser_addr;
    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = 0;
    if((bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr))) < 0) {
        failHandler("bind socket error!");
    }
    socklen_t len = sizeof(struct sockaddr_in);

    fseek(fp, start, SEEK_SET);

    long end_temp;
    while(start < end) {
        if(start + BUFFER_SIZE - 1 < end) {
            end_temp = start + BUFFER_SIZE - 1;
        }else {
            end_temp = end;
        }
        memset(write_buf, 0 , BUFFER_SIZE * sizeof(char));
        strcpy(write_buf->cmd, CMD_DOWNLOAD);
        
        memset(data, 0x00, sizeof(char) * BUFFER_SIZE);
        int rt = fread(data, 1, (end_temp-start+1), fp);
        strcpy(write_buf->file_content, data);

        sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)clent_addr, len);
        start = end_temp + 1; 
    }
    free(thread_param);
    fclose(fp);
    close(server_fd);
    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    checkParam(argc, argv);

// udp basic
    int server_fd;
    struct sockaddr_in ser_addr,clent_addr; 

    if((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        failHandler("create socket error!");
    }

    bzero(&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ser_addr.sin_port = htons(atoi(argv[1]));

    if((bind(server_fd, (struct sockaddr*)&ser_addr, sizeof(ser_addr))) < 0) {
        failHandler("bind socket error!");
    }

    char read_buf[BUFFER_SIZE];
    server_response_t * write_buf = malloc(sizeof(server_response_t));
    socklen_t len = sizeof(struct sockaddr_in);
    char * cmd;
    char * target_file;
    for( ; ; ) {
        memset(write_buf, 0, sizeof(server_response_t));
        bzero(read_buf, BUFFER_SIZE);
        if(recvfrom(server_fd, read_buf, BUFFER_SIZE, 0, (struct sockaddr*)&clent_addr, &len) < 0) {
            continue;
        }
        cmd = strtok(read_buf, " ");
        if(strcmp(cmd, CMD_CHECKFILE) == 0) {
            //check file existing
            target_file = strtok(NULL, " ");
            strcpy(write_buf->cmd, CMD_CHECKFILE);
            strcpy(write_buf->file_content, target_file);
            if (hasFile(target_file)) {
                write_buf->have_file_flag = 1;
                write_buf->file_length = file_length(target_file);
            }else {
                write_buf->have_file_flag = -1;
            }
            
            sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)&clent_addr, len);
            continue;
        }else if(strcmp(cmd, CMD_HANDSHAKE) == 0) {
            //response handshake to show alive
            strcpy(write_buf->cmd, CMD_HANDSHAKE);
            sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)&clent_addr, len);
        }else if(strcmp(cmd, CMD_DOWNLOAD) == 0) {
            //create new port and response that port within a thread
            target_file = strtok(NULL, " ");
            long start = atol(strtok(NULL, " "));
            long end = atol(strtok(NULL, " "));
            thread_param_t * param = malloc(sizeof(thread_param_t));
            memset(param, 0, sizeof(thread_param_t));
            strcpy(param->target_file, target_file);
            param->clent_addr = clent_addr;
            memcpy(&param->clent_addr, &clent_addr, sizeof(struct sockaddr_in));
            param->start = start;
            param->end = end;
            pthread_t t;
            if(pthread_create(&t, NULL, upload, (void *)param) != 0){
			    failHandler("create new thread fail!");
            }
        }
    }
    return 0;
}

void checkParam(int argc, char* argv[]) {
    if(argc != 2) {
        failHandler("Please use server by correct input! usage: ./server <portnumber>");
    }

    if(!isValidPort(argv[1])) {
        failHandler("Please use legal port number!");
    }
}
