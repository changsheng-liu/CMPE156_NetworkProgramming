#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "util.h"
#include "mysocket.h"

void checkParam(int argc, char* argv[]);

int main(int argc, char* argv[])
{
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
    socklen_t len = sizeof(clent_addr);
    char * cmd;
    char * target_file;
    for( ; ; ) {
        memset(write_buf, 0, sizeof(server_response_t));
        bzero(read_buf, BUFFER_SIZE);

        if(recvfrom(server_fd, read_buf, BUFFER_SIZE, 0, (struct sockaddr*)&clent_addr, &len) < 0) {
            failHandler("receive error!");//need modify
        }
        printf("%s\n", read_buf);
        cmd = strtok(read_buf, " ");
        if(strcmp(cmd, CMD_DOWNLOAD) == 0) {
            target_file = strtok(NULL, " ");
            long start = atol(strtok(NULL, " "));
            long end = atol(strtok(NULL, " "));

            strcpy(write_buf->cmd, CMD_DOWNLOAD);
            FILE *fp; 
            if((fp = fopen(target_file,"r")) == NULL) { 
                failHandler("open file error!");
            }

            fseek(fp, start, SEEK_SET);
            long cur = start;
            char data[BUFFER_SIZE];
            memset(data, 0x00, sizeof(char)*BUFFER_SIZE);

            int rt;
            while(end - cur > BUFFER_SIZE-1) {
                rt = fread(data,1,BUFFER_SIZE-1,  fp);
                data[rt] = 0x00;
                strcpy(write_buf->file_content, data);
                write_buf->content_is_end = -1;
                write_buf->content_length = rt;
                cur = cur + rt;
                sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)&clent_addr, len);
                memset(data, 0x00, sizeof(char)*BUFFER_SIZE);
            }
            rt = fread(data, 1, (end-cur+1), fp);
            data[rt] = 0x00;
            strcpy(write_buf->file_content, data);
            write_buf->content_is_end = 1;
            write_buf->content_length = rt;
            sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)&clent_addr, len);
            fclose(fp);

        }else if(strcmp(cmd, CMD_CHECKFILE) == 0) {
            target_file = strtok(NULL, " ");
            strcpy(write_buf->cmd, CMD_CHECKFILE);
            if (hasFile(target_file)) {
                write_buf->have_file_flag = 1;
                write_buf->file_length = file_length(target_file);
            }else {
                write_buf->have_file_flag = -1;
            }
            sendto(server_fd, write_buf, sizeof(server_response_t), 0, (struct sockaddr*)&clent_addr, len);
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